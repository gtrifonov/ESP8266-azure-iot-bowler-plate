#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "../inc/restAPI.h"
#include "../inc/device.h"


ESP8266WebServer _server(80);

struct Led
{
    byte id;
    byte gpio;
    byte status;
} led_resource;

RestApiController::RestApiController()
{
}

void getRequest()
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[200];

    jsonObj["on"] = led_resource.status;
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    _server.send(200, "application/json", JSONmessageBuffer);
}

void postRequest()
{
    StaticJsonBuffer<500> jsonBuffer;
    String post_body = _server.arg("plain");
    Serial.println(post_body);

    JsonObject &jsonBody = jsonBuffer.parseObject(_server.arg("plain"));

    Serial.print("HTTP Method: ");
    Serial.println(_server.method());

    if (!jsonBody.success())
    {
        Serial.println("error in parsin json body");
        _server.send(400);
    }
    else
    {
        if (_server.method() == HTTP_PUT)
        {
            if (jsonBody["id"] > Device::LED_ARRAY_SIZE)
            {
                _server.send(404);
            }
            else
            {
                const int id = jsonBody["id"];
                const byte status = jsonBody["status"];
                Device::setLed(id, status );
                Device::beep();
                _server.send(200);
            }
        }
    }
}

void RestApiController::registerRoutes()
{
    _server.begin();
    Device::initialize();
    _server.on("/rest/leds", HTTP_GET, getRequest);
    _server.on("/rest/leds", HTTP_PUT, postRequest);
}

void RestApiController::handleClient()
{
    _server.handleClient();
}