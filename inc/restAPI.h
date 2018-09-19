
#ifndef REST_API
#define REST_API

#include <ESP8266WebServer.h>


class RestApiController {
public:
    RestApiController(); 
     static void handleClient();    
     static void registerRoutes();
    
};



#endif // REST_API