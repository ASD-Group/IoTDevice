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

void serialPrintCalibStat()    //gets the latest calibration values and prints them via serial
{
    uint8_t system, gyro, accel, mag;
    mpu.getCalibration(&system, &gyro, &accel, &mag);
    Serial.print("CALIB_STAT_SYSTEM:\t");  Serial.println(system, DEC);
    Serial.print("CALIB_STAT_GYR:\t");  Serial.println(gyro, DEC);
    Serial.print("CALIB_STAT_ACC:\t");  Serial.println(accel, DEC);
    Serial.print("CALIB_STAT_MAG:\t");  Serial.println(mag, DEC);
}

void setup() 
{
    Serial.begin(115200);
    wifi_connection("ASUS 18", "597867564", 4210);
    
    while (!mpu.init())
    {
     Serial.println("BNO no connection");
    }
    
    while(!mpu.isFullyCalibrated())    //wait until everything is fully calibrated once....
    {
        serialPrintCalibStat();
        delay(1000);
    }
    mpu.saveOffsets(100);
    Serial.println("Fully Calibrated!"); 
    Serial.println("And calibration settings saved!");
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

  sprintf(buffer,"0,%f,%f,%f,%f",w, x, y, z);
  Serial.println(buffer);
  
  send_packet(buffer);

  delay(50);
}
