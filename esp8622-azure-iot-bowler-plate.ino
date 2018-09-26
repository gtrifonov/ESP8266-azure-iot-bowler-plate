#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include "inc/application.h"


void setup()
{    
    ApplicationController::initialize();
}

void loop()
{
    ApplicationController::loop();
}
