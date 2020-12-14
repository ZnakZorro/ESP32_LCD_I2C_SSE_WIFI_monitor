
#include "Arduino.h"
#include "SPI.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

AsyncWebServer server(80);
AsyncEventSource events("/events");

#include <LiquidCrystal_I2C.h>  //https://github.com/johnrickman/LiquidCrystal_I2C
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;

long   wifiStrange;
String wifiName = "?";

void getSensorReadings(){
    wifiStrange = WiFi.RSSI();
    wifiName    = WiFi.SSID();  
    //Serial.print("#22="); Serial.println(wifiName);Serial.print("#23="); Serial.println(WiFi.RSSI());
}
String IpAddress2String(const IPAddress& ipAddress){
  String k = ".";
  return String(ipAddress[0])+k+String(ipAddress[1])+k+String(ipAddress[2])+k+String(ipAddress[3]);
}

void write2LCD(String text1, String text2){
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print(text1);
     lcd.setCursor(0, 1);
     lcd.print(text2);
}

// Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
           wifiMulti.addAP("login1", "pass1");
           wifiMulti.addAP("login2", "pass2");
           wifiMulti.addAP("login3", "pass3");
          //.................................
    Serial.print("Connecting to WiFi ..");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print('.');  delay(333);
    }
    Serial.println("::");
    Serial.print(WiFi.localIP()); Serial.print("; ");  Serial.print(WiFi.SSID());  Serial.print("; ");   Serial.println(WiFi.RSSI());
        write2LCD(IpAddress2String(WiFi.localIP()),WiFi.SSID());  
    delay(3000);
}

void setup() {
  Serial.begin(115200);
     lcd.init();                    
     lcd.backlight();  
     write2LCD("initWiFi","...");

  initWiFi();
      if (!SPIFFS.begin(true)) {
       Serial.println("An Error while mounting SPIFFS");
      } else{
        Serial.println("OK success mounting SPIFFS");
      }

  // Handle Web Server
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/wifi.webmanifest", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/wifi.webmanifest", "application/manifest+json");
  });
  server.on("/wifi.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/wifi.png", "image/png");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
     client->send("hello!", NULL, millis(), 10000);
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.addHandler(&events);
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    getSensorReadings();
    events.send(String(wifiStrange).c_str(),"wifiStrange",millis());   
    events.send(String(wifiName).c_str(),"wifiName",millis());
    lastTime = millis();
    write2LCD(String(wifiName),String(wifiStrange));
  }
}
