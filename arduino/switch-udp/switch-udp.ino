#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "RAILNET";
const char* password = "it-seminar-2019";

/*const char* ssid = "Das phone";
const char* password = "wwgu1535";*/

/*const char* ssid     = "AndroidAP";
const char* password = "jjwb5593";*/

#define LED_PIN LED_BUILTIN
#define MOTOR_PIN1 D1
#define MOTOR_PIN2 D2
#define MOTOR_PIN3 D3
#define MOTOR_PIN4 D5

#define SWITCH_SET 7
#define SWITCH_STATUS 8
#define LIGHT_SET 9
#define LIGHT_STATUS 10
#define REQUEST_INFO 11
#define DEVICE_INFO 12

#define DEVICE_ID 17

#define MOVE_RANGE 500

WiFiUDP Udp;
unsigned int localUdpPort = 4711;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char outgoingPacket[255];
char replyA[] = "At hall A";
char replyB[] = "At hall B";
char reply[] = "Hello there!";

int deltaCount = 0;
IPAddress remoteIP = IPAddress(0,0,0,0);
uint16_t remotePort = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  digitalWrite(LED_PIN, HIGH);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  digitalWrite(LED_PIN, LOW);
  }


void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.println("recived");
    remoteIP = Udp.remoteIP();
    remotePort = Udp.remotePort();
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 1)
    {
      if (incomingPacket[1] == TRAIN_CONTROL) {
        speed = readInt(incomingPacket, 2);
        Serial.printf("Speed: %d\n", speed);
        applySpeed(speed);
        }
      else if (incomingPacket[1] == TRAIN_BRAKE) {
        speed = 0;
        applySpeed(speed);
        sendTrainInformation(0, 0, 0, 1);
        }
      else if (incomingPacket[1] == TRAIN_DESTINATION) {
        /*speed = readInt(incomingPacket, 2);
        Serial.printf("Speed: %d\n", speed);
        applySpeed(speed);*/
        tripMagnet = readInt(incomingPacket, 2);
        }
      else if (incomingPacket[1] == TRAIN_DIRECTION){
        currentDirection = readInt(incomingPacket,2);
        applySpeed(speed);
        }
      else {
        incomingPacket[len] = 0;
        Serial.printf("UDP packet contents: %s\n", incomingPacket);
        }

      }

    // send back a reply, to the IP address and port we got the packet from
    }
  }

void broadcastExistence() {
  
  }

int readInt(char buffer[], int index) {
  int result = 0;
  for (int i = index + 3; i >= index; i--) {
    result <<= 8;
    result |= buffer[i];
    }
  return result;
  }

int writeInt(char buffer[], int index, int value) {
  for (int i = index; i < index + 4; i++) {
    buffer[i] = value & 255;
    value >>= 8;
    }
  return index + 4;
  }


void sendSwitchStatus(int position) {
  outgoingPacket[0] = SWITCH_STATUS;
  outgoingPacket[1] = position;
  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(outgoingPacket, 2);
  Udp.endPacket();
  }

void sendDeviceInfo() {
  outgoingPacket[0] = DEVICE_INFO;
  outgoingPacket[1] = 1;
  outgoingPacket[2] = DEVICE_ID;
  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(outgoingPacket, 3);
  Udp.endPacket();
  }
