#include "../inc/device.h"
#include "Arduino.h"

int leds[] = {14, 12, 15, 16};
int ledStates[] = {LOW, LOW, LOW, LOW};
int motion_reading = LOW;

byte led1state = LOW;
byte motionState = LOW;

static MOTION_STARTED _motion_started_callback = NULL;
static MOTION_ENDED _motion_ended_callback = NULL;

Device::Device()
{
}

bool Device::initialize()
{

    pinMode(SPEAKERPIN, OUTPUT);
    pinMode(BLUELEDPIN, OUTPUT);
    pinMode(REDLEDPIN, OUTPUT);
    pinMode(MOTIONPIN, INPUT);
    setRedLed(HIGH);
    setBlueLed(HIGH);

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

bool Device::setRedLed(int status)
{
    digitalWrite(REDLEDPIN, status);
}
bool Device::setBlueLed(int status)
{
    digitalWrite(BLUELEDPIN, status);
}
void Device::blink(int pin, int blinkDelay, int numberOfblinks)
{
    int count = 0;

    while (numberOfblinks > count)
    {        
        digitalWrite(pin, LOW);
        delay(blinkDelay);
        digitalWrite(pin, HIGH);
        delay(blinkDelay);
        count++;
    }
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

void Device::registerMotionStartedCallBack(MOTION_STARTED start_callback)
{
    _motion_started_callback = start_callback;
}
void Device::registerMotionEndedCallBack(MOTION_ENDED end_callback)
{
    _motion_ended_callback = end_callback;
}

void Device::detectMotion()
{
    motion_reading = digitalRead(MOTIONPIN); // read sensor value
    delay(100);
    if (motion_reading == HIGH)
    {

        if (motionState == LOW)
        {
            Serial.println("Motion detected!");
            Device::setBlueLed(LOW);            
            motionState = HIGH; // update variable state to HIGH

            if (_motion_started_callback)
            {
                _motion_started_callback();
            }
        }
    }
    else
    {

        if (motionState == HIGH)
        {
            Serial.println("Motion stopped!");
            motionState = LOW; // update variable state to LOW
            Device::setBlueLed(HIGH);
            if (_motion_ended_callback)
            {
                _motion_ended_callback();
            }
        }
    }
}