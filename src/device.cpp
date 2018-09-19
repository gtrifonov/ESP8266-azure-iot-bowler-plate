#include "../inc/device.h"
#include "Arduino.h"

int SPEAKERPIN = 13;
int leds[] = {14, 12, 15, 16};
int ledStates[] = {LOW, LOW, LOW, LOW};

byte led1state = LOW;

Device::Device()
{
}

bool Device::initialize()
{

    pinMode(SPEAKERPIN, OUTPUT);
    for (int i; i = 0; i < Device::LED_ARRAY_SIZE + 1)
    {
        pinMode(ledStates[i], OUTPUT);
        digitalWrite(leds[i], LOW);
    }
    return true;
}

bool Device::setLed(int index, int status)
{
    pinMode(leds[index], OUTPUT);
    digitalWrite(leds[index], status);
    return true;
}

int Device::getLed(int index)
{
    return ledStates[index];
}

void Device::beep()
{
    // http://web.media.mit.edu/~leah/LilyPad/07_sound_code.html
    int x;
    int frequencyInHertz = 400;
    int timeInMilliseconds = 500;
    long delayAmount = (long)(1000000 / frequencyInHertz);
    long loopTime = (long)((timeInMilliseconds * 1000) / (delayAmount * 2));
    for (x = 0; x < loopTime; x++)
    {
        digitalWrite(SPEAKERPIN, HIGH);
        delayMicroseconds(delayAmount);
        digitalWrite(SPEAKERPIN, LOW);
        delayMicroseconds(delayAmount);
    }
}