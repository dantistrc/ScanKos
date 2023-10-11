#include <main.h>

source sr;

void setup()
{
    sr.beginScan();
    sr.beginGSM();
}

void loop()
{
    sr.loopWork();
}


