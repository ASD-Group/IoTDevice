/*
 * file Save_Calibration.ino
 *
 * @ https://github.com/DFRobot/DFRobot_BNO055
 *
 * connect BNO055 I2C interface with your board (please reference board compatibility)
 *
 * Gets the Angular Velocity of the current sensor and prints it out through the serial port.
 *
 *
 * version  V0.1
 * date  2018-1-8
 */
 
#include <Wire.h> 
#include "DFRobot_BNO055.h"

DFRobot_BNO055 mpu;
void setup() 
{
    Serial.begin(115200);
    
    while (!mpu.init())
    {
     Serial.println("ERROR! Unable to initialize the chip!");
     delay(50);
    }
    
    //   mpu.setMode(mpu.eNORMAL_POWER_MODE, mpu.eFASTEST_MODE);
    while(!mpu.isFullyCalibrated())    //wait until everything is fully calibrated once....
    {
        serialPrintCalibStat(); //print the current calibration levels via serial
        delay(1000);
    }
    mpu.saveOffsets(100);
    Serial.println("Fully Calibrated!"); 
    Serial.println("And calibration settings saved!");
    
    //mpu.loadOffsets(100);
    //Serial.println("And calibration settings restored!");
    
    delay(100);
    Serial.println("Read euler angles...");
}

void loop() 
{
  mpu.readEuler();  /* read euler angle */
  
  Serial.print("yaw: "); 
  Serial.print(mpu.EulerAngles.x, 3); 
  Serial.print("  "); 
  
  Serial.print("pitch:"); 
  Serial.print(mpu.EulerAngles.y, 3); 
  Serial.print("  ");
  
  Serial.print("roll: "); 
  Serial.print(mpu.EulerAngles.z, 3); 
  Serial.println("  ");
  
  delay(200);
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
