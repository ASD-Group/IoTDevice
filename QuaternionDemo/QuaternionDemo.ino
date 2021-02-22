#include <Wire.h> 
#include "DFRobot_BNO055.h"

float w, x, y, z;

DFRobot_BNO055 mpu;

void setup() 
{
   Serial.begin(115200);
   while (!mpu.init())
   {
     Serial.println("ERROR! Unable to initialize the chip!");
     delay(30);
   }
   delay(100);
   Serial.println("Read quaternion...");
}

void loop() 
{
  mpu.readQua();

  w = mpu.QuaData.w;
  x = mpu.QuaData.x;
  y = mpu.QuaData.y;
  z = mpu.QuaData.z;
  
  Serial.print(w);
  Serial.print(",");
  Serial.print(x);
  Serial.print(","); 
  Serial.print(y);
  Serial.print(","); 
  Serial.println(z);
   
  delay(50);
}
