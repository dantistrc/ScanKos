#include <SoftwareSerial.h>  // Библиотека програмной реализации обмена по UART-протоколу
SoftwareSerial SIM800(2, 4); // RX, TX

String _response = ""; // Переменная для хранения ответа модуля
uint32_t worktime;
uint32_t globtime;
String sendtime;
int periodmillis;
int onPin = 12;       // Светодиод, подключенный к вход/выходу
uint32_t timerRezult; // Total time
uint32_t timerCount;  // seanse time
uint32_t timerStart;
int onPinOld = LOW;
int day = 0;
int sut;
int Ghour;
int hour;
int min;
int sec;
int workpin;
void setup()
{
    {
        pinMode(onPin, INPUT); // устанавливает режим работы - выход
    }
    Serial.begin(9600); // Скорость обмена данными с компьютером
    SIM800.begin(9600); // Скорость обмена данными с модемом
    Serial.println("Start!");
    const String PHONE = "+79265340402";
    const String incoming = "i";
    // string smessage;
    sendATCommand("AT", true); // Отправили AT для настройки скорости обмена данными
    sut = millis();
    // Команды настройки модема при каждом запуске
    //_response = sendATCommand("AT+CLIP=1", true);  // Включаем АОН
    //_response = sendATCommand("AT+DDET=1", true);  // Включаем DTMF
    _response = sendATCommand("AT+CMGF=1;&W", true); // Включаем текстовый режима SMS (Text mode) и сразу сохраняем значение (AT&W)!
                                                     // sendSMS("+79265340402","Dtimer-" + sendtime + "DA-" + String(day) + "TH-"+ String(Ghour/100) + String(Ghour/10) + String(Ghour%10));
    // sendSMS("+79265340402", "restart-message");
    //!!!!!!!SENDING on RESET!!!!!!!!
}

String sendATCommand(String cmd, bool waiting)
{
    String _resp = "";   // Переменная для хранения результата
    Serial.println(cmd); // Дублируем команду в монитор порта
    SIM800.println(cmd); // Отправляем команду модулю
    if (waiting)
    {                           // Если необходимо дождаться ответа...
        _resp = waitResponse(); // ... ждем, когда будет передан ответ
        // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
        if (_resp.startsWith(cmd))
        { // Убираем из ответа дублирующуюся команду
            _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
        }
        Serial.println(_resp); // Дублируем ответ в монитор порта
    }
    return _resp; // Возвращаем результат. Пусто, если проблема
}

String waitResponse()
{                                     // Функция ожидания ответа и возврата полученного результата
    String _resp = "";                // Переменная для хранения результата
    long _timeout = millis() + 10000; // Переменная для отслеживания таймаута (10 секунд)
    while (!SIM800.available() && millis() < _timeout)
    {
    }; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
    if (SIM800.available())
    {                                // Если есть, что считывать...
        _resp = SIM800.readString(); // ... считываем и запоминаем
    }
    else
    {                                 // Если пришел таймаут, то...
        Serial.println("Timeout..."); // ... оповещаем об этом и...
    }
    return _resp; // ... возвращаем результат. Пусто, если проблема
}
//-------------------------------------------------------------LOOP--------------------------------------------------------------------
void loop()
{
    workpin = digitalRead(onPin); // вкл выкл
                                  //==============================================Суточный таймер==============================================
    if ((millis() - sut) > 86400000 & workpin == LOW)
    {
        Serial.println("sut", sut);
        sut = millis();
        day = day + 1;
        globtime = globtime + worktime;
        Ghour = (globtime / 3600ul); // часы

        // Gmin = (globtime % 3600ul) / 60ul;  // минуты
        // sec = (globtime % 3600ul) % 60ul;  // секунды
        // sendSMS("+79265340402","Dtimer-" + sendtime + "DA-" + String(day) + "TH-"+ String(Ghour/100) + String(Ghour/10) + String(Ghour%10));        // ======SMS======
        // DT.суточный таймер      DA.количество дней всего      TH.время работы тотальное
        worktime = 0; // reset day worktimer
    }

    //....................................................TIMER Works............................................................

    if (workpin == HIGH && onPinOld == LOW)
    {
        onPinOld = HIGH;
        Serial.println("timer on");
        timerStart = millis();
    }
    if (workpin == LOW && onPinOld == HIGH)
    {
        onPinOld = LOW;
        Serial.println("timeroff");
        timerCount = (millis() - timerStart) / 1000ul; // SEC
        worktime = worktime + timerCount;
        hour = (worktime / 3600ul);       // часы
        min = (worktime % 3600ul) / 60ul; // минуты
        sec = (worktime % 3600ul) % 60ul; // секунды

        // uint32_t sec = millis() / 1000ul;      // полное количество секунд
        // int timeHours = (sec / 3600ul);        // часы
        // int timeMins = (sec % 3600ul) / 60ul;  // минуты
        // int timeSecs = (sec % 3600ul) % 60ul;  // секунды
        // timerStart = millis();
        Serial.print(hour, DEC);
        Serial.print(":");
        Serial.print(min, DEC);
        Serial.print(":");
        Serial.println(sec, DEC);

        sendtime = "Time-" + String(hour / 10) + String(hour % 10) + ":" + String(min / 10) + String(min % 10);
    }

    if (SIM800.available())
    {                               // Если модем, что-то отправил...
        _response = waitResponse(); // Получаем ответ от модема для анализа
        _response.trim();           // Убираем лишние пробелы в начале и конце
        Serial.println(_response);  // Если нужно выводим в монитор порта

        //------------------------------------------------Activate RINGS------------------------------------------------------

        if (_response.startsWith("NO CARRIER"))
        {   // Пришло сообщение
            // String worktime =  String(millis(), DEC);             // using a long and a base
            sendtime = "Time-" + String(hour / 10) + String(hour % 10) + ":" + String(min / 10) + String(min % 10);
            sendSMS("+79265340402", "Dtimer-" + sendtime + "DA-" + String(day) + "TH-" + String(Ghour / 100) + String(Ghour / 10) + String(Ghour % 10));
        }
        //....SMS.....
        if (_response.startsWith("+CMGS:"))
        {                                                                       // Пришло сообщение об отправке SMS
            int index = _response.lastIndexOf("\r\n");                          // Находим последний перенос строки, перед статусом
            String result = _response.substring(index + 2, _response.length()); // Получаем статус
            result.trim();                                                      // Убираем пробельные символы в начале/конце

            if (result == "OK")
            { // Если результат ОК - все нормально
                Serial.println("Message was sent. OK");
            }
            else
            { // Если нет, нужно повторить отправку
                Serial.println("Message was not sent. Error");
            }
        }

        //-----------------------------------------------------------------------------------------------------

        //........................................SMS recive...........................................

        if (_response.startsWith("+CMTI:"))
        {                                                                       // Пришло сообщение об отправке SMS
            int index = _response.lastIndexOf(",");                             // Находим последнюю запятую, перед индексом
            String result = _response.substring(index + 1, _response.length()); // Получаем индекс
            result.trim();                                                      // Убираем пробельные символы в начале/конце
            _response = sendATCommand("AT+CMGR=" + result, true);               // Получить содержимое SMS
            parseSMS(_response);                                                // Распарсить SMS на элементы
            sendATCommand("AT+CMGDA=\"DEL ALL\"", true);                        // Удалить все сообщения, чтобы не забивали память модуля
        }
    }

    if (Serial.available())
    {                                // Ожидаем команды по Serial...
        SIM800.write(Serial.read()); // ...и отправляем полученную команду модему
    };
}

void parseSMS(String msg)
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

    Serial.println("Phone: " + msgphone);
    Serial.println("Message: " + msgbody);

    // Далее пишем логику обработки SMS-команд.
    // Здесь также можно реализовывать проверку по номеру телефона
    // И если номер некорректный, то просто удалить сообщение.
    if (msgphone == "+79265340402" && msgbody == "5475")
    {
        // smessage = "counter";
        // wmillis = millis;
        // String worktime =  String(millis(), DEC);//String worktime = wmillis / 60000;
        sendSMS("+79265340402", "sms-" + sendtime);

        Serial.println("SMS Отправлено.");
    }
}

void sendSMS(String phone, String message)
{
    sendATCommand("AT+CMGS=\"" + phone + "\"", true);           // Переходим в режим ввода текстового сообщения
    sendATCommand(message + "\r\n" + (String)((char)26), true); // После текста отправляем перенос строки и Ctrl+Z
}