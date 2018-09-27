
//needed for library
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include "../inc/restAPI.h"
#include "../inc/DevKitMQTTClient.h"
#include "../inc/application.h"
#include "../inc/device.h"

RestApiController rest;

// Times before 2010 (1970 + 40 years) are invalid
#define MIN_EPOCH 40 * 365 * 24 * 3600

bool ApplicationController::initialize()
{
  Serial.begin(9600);
  Serial.setDebugOutput(true);

  Device::initialize();

  time_t epochTime;
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.windows.com");

  while (true)
  {
    epochTime = time(NULL);

    if (epochTime < MIN_EPOCH)
    {
      Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
      delay(2000);
    }
    else
    {
      Serial.print("Fetched NTP epoch time is: ");
      Serial.println(epochTime);
      break;
    }
  }
  

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.autoConnect();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");


  rest.registerRoutes();
  Serial.println("HTTP server started");

  if (DevKitMQTTClient_Init(true) == true)
  {
    Serial.println("Connected to Azure Iot Hub");
    return true;
  }
  else
  {
    Serial.println("Azure Iot Hub connection failed");
    return false;
  }
}

bool ApplicationController::loop()
{
  rest.handleClient();
  DevKitMQTTClient_DoWork();
  delay(50);
}
