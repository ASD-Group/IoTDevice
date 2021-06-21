#include "WiFi.h"
#include "EEPROM.h"
#include "Adafruit_BNO055.h"
#include "Adafruit_Sensor.h"

WiFiUDP udp_slave;
WiFiUDP udp_master;

char master_packet_buffer[255];
char slave_packet_buffer[255];


char* wifi_id;
char* wifi_password;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
adafruit_bno055_offsets_t offsets;
float w, x, y, z;

int packer_counter = 0;
bool is_wifi_connected = false;

void eepromRead()
{
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

void eepromWrite()
{     
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

void acess_point_setup(char* access_point_id, char* access_point_password, int slave_port)
{
  WiFi.softAP(access_point_id, access_point_password, 1, 0, 10);
  udp_slave.begin(slave_port);
}

void eeprom_wifi_write(char* ssid, char* password)
{
  
  EEPROM.write(13, 1);

  // start adderss for ssid
  int wifi_id_addr = 14; 

  // write the length of ssid
  byte ssid_len = strlen(ssid);
  EEPROM.write(wifi_id_addr, ssid_len);

   

  // write ssid
  for(int i = 0; i < ssid_len; i++){
    EEPROM.write(wifi_id_addr + i + 1, ssid[i]);
    
  }
  

  // start adderss for password
  int wifi_password_addr = wifi_id_addr + ssid_len + 1;

  // write the length of password
  byte password_len = strlen(password);
  EEPROM.write(wifi_password_addr, password_len);

  

  // write password
  for(int i = 0; i < password_len; i++){
    EEPROM.write(wifi_password_addr + i + 1, password[i]);
    
  }
  EEPROM.commit();
}

void eeprom_wifi_read()
{
  
  //get the length of ssid from eeprom
  int wifi_id_addr = 14; 
  int ssid_len = EEPROM.read(wifi_id_addr);
  
  //get the length of password from eeprom
  int wifi_password_addr = wifi_id_addr + ssid_len + 1;
  int pass_len = EEPROM.read(wifi_password_addr);

  //initialize ssid and password

  wifi_id = new char[ssid_len];
  wifi_password = new char[pass_len];

  wifi_id[ssid_len] = '\0';
  wifi_password[pass_len] = '\0';

  //get wifi_id from eeprom
  for(int i = 0; i < ssid_len; i++){
    wifi_id[i] = EEPROM.read(wifi_id_addr + i + 1);
  }

  
  //get wifi_password from eeprom  
  for(int i = 0; i < pass_len; i++){
    wifi_password[i] = EEPROM.read(wifi_password_addr + i + 1);
  }

  Serial.println(wifi_id);
  Serial.println(wifi_password);
}

void wifi_setup(int master_port)
{
  
  if(EEPROM.read(13) != 1){
    return;
  }

  eeprom_wifi_read();


  unsigned long start_time = millis();

  WiFi.begin(wifi_id, wifi_password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    if((millis() - start_time) / 1000 > 15){
      return;
    }
    delay(100);
  }

  is_wifi_connected = true;
  udp_master.begin(master_port);

  
  Serial.printf("ip %s port %d\n", WiFi.localIP().toString().c_str(), master_port);
}

void bno_eeprom(){
  EEPROM.begin(100); 
  if(EEPROM.read(90) == 1){
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
    delay(100);
    }
    bno.getOffsets(offsets);
    eepromWrite();
  }  
}

void setup()
{
  Serial.begin(2000000);
  Wire.begin(21, 22);

  acess_point_setup("P2SERVER", "P2SERVER", 9999);

  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(true);
  }

  bno_eeprom();

  wifi_setup(8888);

  while(true)
  {
    if(is_wifi_connected)
    {
      int master_packet_size = udp_master.parsePacket();
      
      if(master_packet_size)
      {
        udp_master.read(master_packet_buffer, 255);
      }
      
      int slave_packet_size = udp_slave.parsePacket();
      
      if(slave_packet_size)
      {
        udp_slave.read(slave_packet_buffer, 255);
        
        if(packer_counter == 0){
          char quat_buffer[16];
          imu::Quaternion quat = bno.getQuat();
          quat.normalize();

          udp_master.beginPacket();
          udp_master.printf("{\"id\":\"p2\",\"sensors\":[{\"id\":\"B\",\"w\":");
          sprintf(quat_buffer,"%f",quat.w());
          udp_master.printf(quat_buffer);
          udp_master.printf(",\"x\":");
          sprintf(quat_buffer,"%f",quat.x());
          udp_master.printf(quat_buffer);
          udp_master.printf(",\"y\":");
          sprintf(quat_buffer,"%f",quat.y());
          udp_master.printf(quat_buffer);
          udp_master.printf(",\"z\":");
          sprintf(quat_buffer,"%f",quat.z());
          udp_master.printf(quat_buffer);
          udp_master.printf("},");
        }
        else if(packer_counter == 9)
        {
      
      
          udp_master.printf(slave_packet_buffer);
          udp_master.printf("]}");

          udp_master.endPacket();
          
          packer_counter = -1;
        }
        else {
          udp_master.printf(slave_packet_buffer);
          udp_master.printf(",");
        }
        packer_counter++;
        memset(slave_packet_buffer, 0, sizeof(slave_packet_buffer));  
      }
    }
    else
    {
      int slave_packet_size = udp_slave.parsePacket();
      
      if(slave_packet_size)
      {
        udp_slave.read(slave_packet_buffer, 255);

        char* piece = strtok(slave_packet_buffer, "\"");

    for(int i = 0; piece != NULL; i++){
       if(i == 3){
        wifi_id = piece;
        Serial.print(wifi_id);
        Serial.print(" ");
        Serial.println(strlen(wifi_id));
       }
       else if(i == 7){
        wifi_password = piece;
        Serial.print(wifi_password);
        Serial.print(" ");
        Serial.println(strlen(wifi_password));
       }
        piece = strtok(NULL, "\"");
    }

    eeprom_wifi_write(wifi_id, wifi_password);
    wifi_setup(8888);
    

    memset(slave_packet_buffer, 0, sizeof(slave_packet_buffer));
      }
    }
  }
}

void loop(){
  
}
