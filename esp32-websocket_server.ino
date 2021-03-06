#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "SPIFFS.h"

WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

char* ssid = "";
char* password = "";

uint8_t ledPin = 2;
bool sts = true;
uint16_t adcPin = 34; // default adc pin is 34
bool paused = true;

void setup()
{
  SPIFFS.begin();
  pinMode(ledPin, OUTPUT);
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",serveIndexFile);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  server.handleClient();
  if (paused == false){
    uint16_t adcVal = analogRead(adcPin);
    String toSend = "g" + String(adcVal);
    webSocket.broadcastTXT(toSend);
  }
}

void serveIndexFile()
{
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    if(payload[0] == '!'){
      digitalWrite(ledPin, sts);
      sts = !sts;
    }
    else if (payload[0] == 'p'){
      paused = !paused;
    }
    else if (payload[0] == '@'){
      adcPin = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
    }
    else{
      for(int i = 0; i < length; i++){
        Serial.print((char) payload[i]);
        Serial.println();
      }
    }
  }
}
