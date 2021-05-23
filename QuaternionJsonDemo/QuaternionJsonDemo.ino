#include <WiFi.h>

#include <Adafruit_BNO055.h>

uint16_t PORT = 8888;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 20;

WiFiUDP Udp;

char packet[255];
char quat_buffer[10];

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);


float w, x, y, z;

void wifi_connection(const char* ssid, const char* password)
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  
  Udp.begin(PORT);
  Serial.printf("ip %s port %d\n", WiFi.localIP().toString().c_str(), PORT);
}

void bno_initalization()
{
  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(true);
  }
}

void udp_read_packet()
{
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packet, 255);
    if (len > 0){
      packet[len] = '\0';
    }
  Serial.println(packet);
  }
}


void setup()
{
  Serial.begin(115200);
  
  // Only for testing chinese esp
  Wire.begin(16, 17);

  wifi_connection("ASUS 18", "597867564");
  bno_initalization();
}

void loop()
{
  udp_read_packet();
  
  imu::Quaternion quat = bno.getQuat();
  quat.normalize();

  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.printf("{\"w\":");
  sprintf(quat_buffer, "%f", quat.w());
  Udp.printf(quat_buffer);
  Udp.printf(",\"x\":");
  sprintf(quat_buffer, "%f", quat.x());
  Udp.printf(quat_buffer);
  Udp.printf(",\"y\":");
  sprintf(quat_buffer, "%f", quat.y());
  Udp.printf(quat_buffer);
  Udp.printf(",\"z\":");
  sprintf(quat_buffer, "%f", quat.z());
  Udp.printf(quat_buffer);
  Udp.printf("}");
  
  Udp.endPacket();
  

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
