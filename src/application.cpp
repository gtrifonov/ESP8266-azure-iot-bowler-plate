#include "../inc/application.h"


//needed for library
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include "../inc/restAPI.h"

RestApiController rest;

bool ApplicationController::initialize()
{
  Serial.begin(9600);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect("GEOTRIAC");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  rest.registerRoutes();
  Serial.println("HTTP server started");
}

bool ApplicationController::loop()
{
  rest.handleClient();
  delay(50);
}