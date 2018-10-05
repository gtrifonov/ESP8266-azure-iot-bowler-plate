#ifndef DEVICE
#define DEVICE

typedef void (*MOTION_STARTED)();
typedef void (*MOTION_ENDED)();

class Device {

public:
    static const int LED_ARRAY_SIZE = 4;
    static const int SPEAKERPIN = 13;
    static const int REDLEDPIN = 0;
    static const int BLUELEDPIN = 2;
    static const int MOTIONPIN = 4;
         
    Device();
    static bool initialize();

    // Control user wired leds 
    static bool setLed(int index, int status);
    static int getLed(int index);

    // Adafruit Feather HUZZAH ESP8266 board leds
    static bool setRedLed(int status);
    static bool setBlueLed(int status);
    static void blink(int pin, int delay, int numberOfblinks);
    static void beep();
    static void detectMotion();
    static void registerMotionStartedCallBack(MOTION_STARTED callback);
    static void registerMotionEndedCallBack(MOTION_ENDED callback);
};

#endif // DEVICE