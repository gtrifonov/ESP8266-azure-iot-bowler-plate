#ifndef DEVICE
#define DEVICE

class Device {
public:
    static const int LED_ARRAY_SIZE = 4;
    Device();
    static bool initialize();

    // Control user wired leds 
    static bool setLed(int index, int status);
    static int getLed(int index);

    // Adafruit Feather HUZZAH ESP8266 board leds
    static bool setRedLed(int status);
    static bool setBlueLed(int status);

    static void beep();
};

#endif // DEVICE