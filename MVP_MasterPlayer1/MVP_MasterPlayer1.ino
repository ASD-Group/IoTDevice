#include <WiFi.h>
#include <Adafruit_BNO055.h>
#include <ArduinoJson.h>

#define ACCESS_POINT_ID "P1SERVER"
#define ACCESS_POINT_PASSWORD "P1SERVER"
#define WIFI_ID "ASUS 18"
#define WIFI_PASSWORD "597867564"
#define UDP_SENDING_DELAY 20
#define BNO_ADDRESS 0x28
#define COSTUME_ID "player1"
#define SERNSOR_ID "B"

WiFiUDP udp_master; 
WiFiUDP udp_slave;

char master_packet_buffer[255];
char slave_packet_buffer[255];

unsigned int master_local_port = 8888;
unsigned int slave_local_port = 9999;

StaticJsonDocument<256> data_json;

bool LH = false;
bool LLA = false;
bool LUA = false;

Adafruit_BNO055 bno;
float w, x, y, z;
char quat_buffer[16];


void setup()
{
  Serial.begin(115200);

  Wire.begin(21, 22);
  
  // WiFi connection
  WiFi.begin(WIFI_ID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  udp_master.begin(master_local_port);
  Serial.printf("ip %s port %d\n", WiFi.localIP().toString().c_str(), master_local_port);

  // Access point setup
  WiFi.softAP(ACCESS_POINT_ID, ACCESS_POINT_PASSWORD);
  udp_slave.begin(slave_local_port);

  // BNO055 init
  bno = Adafruit_BNO055(55, BNO_ADDRESS);
  
  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(true);
  }
}

void loop()
{

   // Quaternions
  imu::Quaternion quat = bno.getQuat();
  quat.normalize();

  w = quat.w();
  x = quat.x();
  y = quat.y();
  z = quat.z();

  // Read Master UDP
  int master_packet_size = udp_master.parsePacket();
  
  if (master_packet_size) {
    int len = udp_master.read(master_packet_buffer, 255);
    Serial.println(master_packet_buffer);
  }

  // Read Slave UDP
  int slave_packet_size = udp_slave.parsePacket();
  
  if (slave_packet_size) {
    char slave_packet_data[255];
    char lh_data[255];
    char lla_data[255];
    
    int len = udp_slave.read(slave_packet_buffer, 255);

    
    
    
    strcpy(slave_packet_data, slave_packet_buffer);

    // Deserialize the JSON document  
    DeserializationError error = deserializeJson(data_json, slave_packet_buffer);
    Serial.println(slave_packet_data);
    

    if(error){
      Serial.println("Ooops, deserialize Json error!");
      return;
    }

    if(data_json["id"] == "LH"){
      strcpy(lh_data, slave_packet_data);
      LH = true;
      
    } // else if

    if(data_json["id"] == "LLA"){
      strcpy(lla_data, slave_packet_data);
      LLA = true;
      
    }

    
    
    if(LH == true && LLA == true){
      
      udp_master.beginPacket();
      
      udp_master.printf("{\"id\":\"");
      udp_master.printf(COSTUME_ID);
      udp_master.printf("\",\"sensors\":[");
      udp_master.printf("{\"id\":\"");
      udp_master.printf(SERNSOR_ID);
      udp_master.printf("\",\"w\":");
      sprintf(quat_buffer,"%f",w);
      udp_master.printf(quat_buffer);
      udp_master.printf(",\"x\":");
      sprintf(quat_buffer,"%f",x);
      udp_master.printf(quat_buffer);
      udp_master.printf(",\"y\":");
      sprintf(quat_buffer,"%f",y);
      udp_master.printf(quat_buffer);
      udp_master.printf(",\"z\":");
      sprintf(quat_buffer,"%f",z);
      udp_master.printf(quat_buffer);
      udp_master.printf("},");
      udp_master.printf(lh_data);
      udp_master.printf(",");
      udp_master.printf(lla_data);
      udp_master.printf("]}");
      
      
      udp_master.endPacket();
      
      LH = false;
      LLA = false;
      
    }
    memset(slave_packet_buffer, 0, sizeof(slave_packet_buffer));
  }
  
  delay(UDP_SENDING_DELAY);
}
