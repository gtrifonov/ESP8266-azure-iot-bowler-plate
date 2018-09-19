#ifndef DEVICE
#define DEVICE

class Device {
public:
    static const int LED_ARRAY_SIZE = 4;
    Device();
    static bool initialize();
    static bool setLed(int index, int status);
    static int getLed(int index);
    static void beep();
};

#endif // DEVICE