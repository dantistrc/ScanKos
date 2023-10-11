#include "Source.h"

SoftwareSerial SIM800(2, 4); // RX, TX

source::source(/* args */)
{
}

source::~source()
{
}

String source::waitResponse()
{                                              // Функция ожидания ответа и возврата полученного результата
    String _resp = "";                         // Переменная для хранения результата
    unsigned long _timeout = millis() + 10000; // Переменная для отслеживания таймаута (10 секунд)
    while (!SIM800.available() && millis() < _timeout)
    {
    }; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
    if (SIM800.available())
    {                                // Если есть, что считывать...
        _resp = SIM800.readString(); // ... считываем и запоминаем
    }
    else
    { // Если пришел таймаут, то...
      // Serial.println("Timeout..."); // ... оповещаем об этом и...
    }
    return _resp; // ... возвращаем результат. Пусто, если проблема
}

String source::sendATCommand(String cmd, bool waiting)
{
    String _resp = ""; // Переменная для хранения результата
    // Serial.println(cmd); // Дублируем команду в монитор порта
    SIM800.println(cmd); // Отправляем команду модулю
    if (waiting)
    {                           // Если необходимо дождаться ответа...
        _resp = waitResponse(); // ... ждем, когда будет передан ответ
        // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
        if (_resp.startsWith(cmd))
        { // Убираем из ответа дублирующуюся команду
            _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
        }
        // Serial.println(_resp); // Дублируем ответ в монитор порта
    }
    return _resp; // Возвращаем результат. Пусто, если проблема
}

void source::sendSMS(String phone, String message)
{
    sendATCommand("AT+CMGS=\"" + phone + "\"", true);           // Переходим в режим ввода текстового сообщения
    sendATCommand(message + "\r\n" + (String)((char)26), true); // После текста отправляем перенос строки и Ctrl+Z
}

void source::parseSMS(String msg)
{
    String msgheader = "";
    String msgbody = "";
    String msgphone = "";

    msg = msg.substring(msg.indexOf("+CMGR: "));
    msgheader = msg.substring(0, msg.indexOf("\r"));

    msgbody = msg.substring(msgheader.length() + 2);
    msgbody = msgbody.substring(0, msgbody.lastIndexOf("OK"));
    msgbody.trim();

    int firstIndex = msgheader.indexOf("\",\"") + 3;
    int secondIndex = msgheader.indexOf("\",\"", firstIndex);
    msgphone = msgheader.substring(firstIndex, secondIndex);

    // Serial.println("Phone: " + msgphone);
    // Serial.println("Message: " + msgbody);

    // Далее пишем логику обработки SMS-команд.
    // Здесь также можно реализовывать проверку по номеру телефона
    // И если номер некорректный, то просто удалить сообщение.
    if (msgphone == PHONE && msgbody == "5475")
    {
        finalSMS("sms:> ");

        // Serial.println("SMS Отправлено.");
    }
}

void source::loopWork()
{
    workpin = digitalRead(onPin); // вкл выкл

    //....................................................TIMER Works............................................................

    if (workpin == HIGH && onPinOld == LOW)
    {
        onPinOld = HIGH;
        // Serial.println("timer on");
        timerStart = millis();
    }

    if (workpin == LOW && onPinOld == HIGH)
    {
        wtime();
    }

    if (SIM800.available())
    {
        // Если модем, что-то отправил...
        _response = waitResponse(); // Получаем ответ от модема для анализа
        _response.trim();           // Убираем лишние пробелы в начале и конце
        // Serial.println(_response);  // Если нужно выводим в монитор порта

        //------------------------------------------------Activate RINGS------------------------------------------------------

        if (_response.startsWith("NO CARRIER"))
        { // Пришло сообщение

            finalSMS("ring:> ");
        }
        //....SMS.....
        if (_response.startsWith("+CMGS:"))
        {                                                                       // Пришло сообщение об отправке SMS
            int index = _response.lastIndexOf("\r\n");                          // Находим последний перенос строки, перед статусом
            String result = _response.substring(index + 2, _response.length()); // Получаем статус
            result.trim();                                                      // Убираем пробельные символы в начале/конце

            // if (result == "OK")
            // { // Если результат ОК - все нормально
            //   // Serial.println("Message was sent. OK");
            // }
            // else
            // { // Если нет, нужно повторить отправку
            //   // Serial.println("Message was not sent. Error");
            // }
        }

        //-----------------------------------------------------------------------------------------------------

        //........................................SMS recive...........................................

        if (_response.startsWith("+CMTI:"))
        {
            int index = _response.lastIndexOf(",");                             // Находим последнюю запятую, перед индексом
            String result = _response.substring(index + 1, _response.length()); // Получаем индекс
            result.trim();                                                      // Убираем пробельные символы в начале/конце
            _response = sendATCommand("AT+CMGR=" + result, true);               // Получить содержимое SMS
            parseSMS(_response);                                                // Распарсить SMS на элементы
            sendATCommand("AT+CMGDA=\"DEL ALL\"", true);                        // Удалить все сообщения, чтобы не забивали память модуля
        }
    }

    // if (Serial.available())
    // {                                // Ожидаем команды по Serial...
    //     SIM800.write(Serial.read()); // ...и отправляем полученную команду модему
    // }

    //==============================================Суточный таймер==============================================

    if ((millis() - sut) > day24 && workpin == LOW)
    {
        sut = millis();

        day = day + 1;

        finalSMS("sutki:> ");

        worktime = 0; // reset day worktimer
    }
}

void source::finalSMS(String definition)
{

    hour = (globtime / 3600ul);       // часы
    min = (globtime % 3600ul) / 60ul; // минуты
    sec = (globtime % 3600ul) % 60ul; // секунды
                                      // Serial.print("day: ");
                                      // Serial.print(day);

    // Serial.print(" hour: ");
    // Serial.print(hour);

    // Serial.print(" min: ");
    // Serial.print(min);

    // Serial.print(" sec: ");
    // Serial.println(sec);

    // Serial.print("worktime: ");
    // Serial.print(worktime / 3600ul);
    // Serial.print(" : ");
    // Serial.print((worktime % 3600ul) / 60ul);
    // Serial.print(" : ");
    // Serial.println((worktime % 3600ul) % 60ul);

    String d = String(day);
    String hh = String(hour);
    String mm = String(min);

    String dateTime = d + " day " + hh + " hour " + mm + " min ";

    String h = String(worktime / 3600ul);
    String m = String((worktime % 3600ul) / 60ul);
    String s = String((worktime % 3600ul) % 60ul);

    String time = "wtime " + h + ":" + m + ":" + s;

    sendSMS(PHONE, definition + dateTime + time); // ======SMS======
}

void source::beginScan()
{
    pinMode(onPin, INPUT); // устанавливает режим работы - выход
}

void source::beginGSM()
{
    // Serial.begin(9600); // Скорость обмена данными с компьютером

    SIM800.begin(9600); // Скорость обмена данными с модемом

    // Serial.println("Start!");

    sut = millis();

    sendATCommand("AT", true); // Отправили AT для настройки скорости обмена данными
    // // Команды настройки модема при каждом запуске
    _response = sendATCommand("AT+CMGF=1;&W", true); // Включаем текстовый режима SMS (Text mode) и сразу сохраняем значение (AT&W)!
                                                     // sendSMS("+79265340402", "restart-message");
                                                     // !!!!!!!SENDING on RESET!!!!!!!!
}
void source::wtime()
{
    onPinOld = LOW;
    // Serial.println("timeroff");
    timerCount = (millis() - timerStart) / 1000ul; // SEC
    worktime += timerCount;
    globtime += timerCount;
    hour = (worktime / 3600ul);       // часы
    min = (worktime % 3600ul) / 60ul; // минуты
    sec = (worktime % 3600ul) % 60ul; // секунды

    // Serial.println(timerCount);
    // Serial.println(worktime);
    // Serial.println(globtime);
    // Serial.println();

    // Serial.print("on-off ");
    // Serial.print(hour, DEC);
    // Serial.print(":");
    // Serial.print(min, DEC);
    // Serial.print(":");
    // Serial.println(sec, DEC);
}
