#include <Wire.h> 
#include "DFRobot_BNO055.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
DFRobot_BNO055 mpu;

float x, y, z, w;
char buffer[255];
char incomingPacket[255];

void wifi_connection(const char* ssid, const char* password, unsigned int localUdpPort)
{
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void send_packet(char text[255])
{
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(text);
  Udp.endPacket();
}

void get_packet()
{
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize); 
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = '\0';
    }
    Serial.print("Packet received: ");
    Serial.println(incomingPacket);
  }
}

void setup()
{
  Serial.begin(115200);
   wifi_connection("ASUS 18", "597867564", 4210);

   while (!mpu.init())
   {
    Serial.println("mpu no connection");
   }
}

void loop()
{
  get_packet();
  mpu.readQua();
  buffer[0] = '\0';

  w = mpu.QuaData.w;
  x = mpu.QuaData.x;
  y = mpu.QuaData.y;
  z = mpu.QuaData.z;

  sprintf(buffer,"%f,%f,%f,%f",w, x, y, z);
  Serial.println(buffer);
  
  send_packet(buffer);

  delay(50);
}
