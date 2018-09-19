## Simple rest api boiler plate code for arduino wifi compatible devices ##

This repository contains wifi arduino compatible device which exposes access to digital IO (led) through rest api

### Supported features ###
 - WIFI manamement WEB UI to register WIFI network SSID and password. Uses WiFiManager library https://github.com/tzapu/WiFiManager
 - REST API to set LED lights on/off.

### Required libraries to install ###
 - ArduinoJson https://arduinojson.org/
 - WiFiManager  https://github.com/tzapu/WiFiManager
  - ESP8266WebServer

### TODO: ###
- move device to globals
- add AZURE IOTHUB connectivity