#include <Wire.h> 
#include "DFRobot_BNO055.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
DFRobot_BNO055 bno;

float w, x, y, z;
char buffer[255];
char incomingPacket[255];

void WiFiConnection(const char* ssid, const char* password, unsigned int localUdpPort){
      
      Serial.printf("Connecting to %s ", ssid);
      WiFi.begin(ssid, password);
      while(WiFi.status() != WL_CONNECTED){
            delay(500);
      }
      Udp.begin(localUdpPort);
      Serial.printf("ip %s port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void SendPacket(char text[255]){
      
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(text);
      Udp.endPacket();
}

void GetPacket(){

      int packetSize = Udp.parsePacket();
      if (packetSize) {
            int len = Udp.read(incomingPacket, 255);
            if (len > 0){
                  incomingPacket[len] = '\0';
            }
            Serial.println(incomingPacket);
      }
}

void PrintCalibration(){

      uint8_t system, gyro, accel, mag;
      bno.getCalibration(&system, &gyro, &accel, &mag);
      buffer[0] = '\0';
      sprintf(buffer, "%s%d%s%d%s%d", "GYRO: ", gyro, " ACCEL: ", accel, " MAG: ", mag);
      Serial.println(buffer);
      SendPacket(buffer);
}

void setup(){

      Serial.begin(115200);
      WiFiConnection("ASUS 18", "597867564", 4210);
      GetPacket();

      while (!bno.init()){
            buffer[0] = '\0';
            sprintf(buffer, "BNO no connection");
            Serial.println(buffer);
            SendPacket(buffer);
            delay(500);
      }

      bno.loadOffsets(100);
      int counter = 0;
      while(!bno.isFullyCalibrated()){
        PrintCalibration();
        delay(1000);
        counter++;
      }
      if(counter > 3){
        bno.saveOffsets(100);
      }
              
}

void loop(){

      GetPacket();
      bno.readQua();
      buffer[0] = '\0';

      w = bno.QuaData.w;
      x = bno.QuaData.x;
      y = bno.QuaData.y;
      z = bno.QuaData.z;

      sprintf(buffer,"0,%f,%f,%f,%f",w, x, y, z);
      Serial.println(buffer);

      SendPacket(buffer);

      delay(50);
}
