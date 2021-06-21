#include <WiFi.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <utility/imumaths.h>
#include <EEPROM.h>

#define ACCESS_POINT_ID "P2SERVER"
#define ACCESS_POINT_PASSWORD "P2SERVER"
#define UDP_SENDING_DELAY 50
#define BNO_ADDRESS 0x28

//#define SERNSOR_ID "LLA"
//#define SERNSOR_ID "RLA"
//#define SERNSOR_ID "LUA"
//#define SERNSOR_ID "RUA"
//#define SERNSOR_ID "LLL"
//#define SERNSOR_ID "RLL"
//#define SERNSOR_ID "LUL"
//#define SERNSOR_ID "RUL"
#define SERNSOR_ID "H"


#define EEPROM_SIZE 100

uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;
int eepromHasData = 0;

WiFiUDP udp_slave;

IPAddress ipServer(192, 168, 4, 1); // default server ip

//IPAddress ipClient(192, 168, 4, 13);
//IPAddress ipClient(192, 168, 4, 14);
//IPAddress ipClient(192, 168, 4, 15);
//IPAddress ipClient(192, 168, 4, 16);
//IPAddress ipClient(192, 168, 4, 17);
//IPAddress ipClient(192, 168, 4, 18);
//IPAddress ipClient(192, 168, 4, 19);
//IPAddress ipClient(192, 168, 4, 20);
IPAddress ipClient(192, 168, 4, 21);

IPAddress Subnet(255, 255, 255, 0);

unsigned int slave_port = 9999;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
adafruit_bno055_offsets_t offsets;

char quat_buffer[16]; 
float w, x, y, z;
unsigned long _time;

void eepromRead(){
  offsets.accel_offset_x = EEPROM.read(91);
  offsets.accel_offset_y = EEPROM.read(92);
  offsets.accel_offset_z = EEPROM.read(93);

  offsets.mag_offset_x = EEPROM.read(94);
  offsets.mag_offset_y = EEPROM.read(95);
  offsets.mag_offset_z = EEPROM.read(96);

  offsets.gyro_offset_x = EEPROM.read(97);
  offsets.gyro_offset_y = EEPROM.read(98);
  offsets.gyro_offset_z = EEPROM.read(99);

  offsets.accel_radius = EEPROM.read(100);

  offsets.mag_radius = EEPROM.read(101);
}

void eepromWrite(){     
  EEPROM.write(90, 1);
  EEPROM.write(91, offsets.accel_offset_x);
  EEPROM.write(92, offsets.accel_offset_y);
  EEPROM.write(93, offsets.accel_offset_z);

  EEPROM.write(94, offsets.mag_offset_x);
  EEPROM.write(95, offsets.mag_offset_y);
  EEPROM.write(96, offsets.mag_offset_z);

  EEPROM.write(97, offsets.gyro_offset_x);
  EEPROM.write(98, offsets.gyro_offset_y);
  EEPROM.write(99, offsets.gyro_offset_z);

  EEPROM.write(100, offsets.accel_radius);

  EEPROM.write(101, offsets.mag_radius);
  EEPROM.commit();
  Serial.println("Data in EEPROM");
}

void setup()
{
  Serial.begin(2000000);

  Wire.begin(16, 17);

  // Connecting to access point
  WiFi.begin(ACCESS_POINT_ID, ACCESS_POINT_PASSWORD);
  WiFi.mode(WIFI_STA);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
  }

  WiFi.config(ipClient, ipServer, Subnet);

  // UDP begin
  udp_slave.begin(slave_port);


  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(true);
  }

  EEPROM.begin(EEPROM_SIZE); 
  eepromHasData = EEPROM.read(90);
  Serial.println(eepromHasData);
  if(eepromHasData == 1){
    eepromRead();
    bno.setOffsets(offsets);
    Serial.println("Setting offsets succesfull!");
  }
  else{
    while(!bno.isFullyCalibrated()){
    uint8_t system, gyro, accel, mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);
    Serial.println();
    Serial.print("Calibration: Sys=");
    Serial.print(system);
    Serial.print(" Gyro=");
    Serial.print(gyro);
    Serial.print(" Accel=");
    Serial.print(accel);
    Serial.print(" Mag=");
    Serial.println(mag);

    Serial.println("--");
    delay(BNO055_SAMPLERATE_DELAY_MS);
    }
    bno.getOffsets(offsets);
    eepromWrite();
  }  
  
}

void loop()
{
  _time = millis();

  // Quaternions
  imu::Quaternion quat = bno.getQuat();
  quat.normalize();

  w = quat.w();
  x = quat.x();
  y = quat.y();
  z = quat.z();

  Serial.print(w);
  Serial.print(" ");
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);

  // Sending UDP
  udp_slave.beginPacket(ipServer, slave_port);

  // Generate JSON
  udp_slave.printf("{\"id\":\"");
  udp_slave.printf(SERNSOR_ID);
  udp_slave.printf("\",\"w\":");
  sprintf(quat_buffer,"%f",w);
  udp_slave.printf(quat_buffer);
  udp_slave.printf(",\"x\":");
  sprintf(quat_buffer,"%f",x);
  udp_slave.printf(quat_buffer);
  udp_slave.printf(",\"y\":");
  sprintf(quat_buffer,"%f",y);
  udp_slave.printf(quat_buffer);
  udp_slave.printf(",\"z\":");
  sprintf(quat_buffer,"%f",z);
  udp_slave.printf(quat_buffer);
  udp_slave.printf("}");
  udp_slave.endPacket();

  while(millis() - _time < 20){
    
  }
}
