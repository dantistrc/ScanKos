#ifndef SOURCE_H
#define SOURCE_H

#include "Arduino.h"
#include <SoftwareSerial.h> // Библиотека програмной реализации обмена по UART-протоколу

class source
{
private:
    /* data */
public:
    // VARIABLES

    String _response = ""; // Переменная для хранения ответа модуля

    uint32_t worktime;
    uint32_t globtime;

    String sendtime;

    byte onPin = 12;      // Светодиод, подключенный к вход/выходу

    uint32_t timerCount;  // seanse time
    uint32_t timerStart;

    bool onPinOld = LOW;

    unsigned int day = 0;
    unsigned long sut;

    unsigned int hour;
    unsigned int min;
    unsigned int sec;

    unsigned int workpin;

    const String PHONE = "+79265340402";

    unsigned long day24 = 86400000;

    // FUNCTIONS

    void loopWork();
    void beginScan();
    void beginGSM();
    String waitResponse();
    String sendATCommand(String cmd, bool waiting);
    void sendSMS(String phone, String message);
    void parseSMS(String msg);
    void wtime();

    void finalSMS(String definition);

    // ANOTHER

    source(/* args */);
    ~source();
};

#endif