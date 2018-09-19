
#ifndef APPLICATION_CONTROLLER
#define APPLICATION_CONTROLLER

// SINGLETON
class ApplicationController {
    ApplicationController() { }
public:
    static bool initialize();
    static bool loop();
};

#endif // APPLICATION_CONTROLLER