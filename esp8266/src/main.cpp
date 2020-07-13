#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>
#include <settings.h>
#include <ESP8266WiFi.h>

long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

char msgString[128];

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

WiFiServer wifiServer(5001);

void setupWifi() {
  delay(500);
  WiFi.begin(WLAN_SSID, WLAN_SECRET);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(F("Connecting to WIFI.."));
  }
 
  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());
 
  wifiServer.begin();
}

void setupCan() {
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);
  
  Serial.println("MCP2515 Remote output ...");
}

void setup()
{
  Serial.begin(115200);
  setupWifi();
  setupCan();  
}

void handleCanMessages(WiFiClient& client) {
  if(!digitalRead(CAN0_INT)) 
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);              
    if((rxId & 0x80000000) == 0x80000000)          
      sprintf(msgString, "can0 %.8lX [%1d]", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "can0 %.3lX [%1d]", rxId, len);
  
    client.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){   
      sprintf(msgString, " REMOTE REQUEST FRAME");
      client.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " %.2X", rxBuf[i]);
        client.print(msgString);
      }
    }
        
    client.println();
  }
}

void loop()
{
  WiFiClient client = wifiServer.available();
  if (client) {
    Serial.println("Client connected!");
    while (client.connected()) {
      handleCanMessages(client);
      yield();
    }
 
    client.stop();
    Serial.println("Client disconnected");
  }  
}

