#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>      // Network Time Protocol Client, used to validate certificates
#include <WiFiUdp.h>        // UDP to communicate with the NTP server
#include <SoftwareSerial.h>
#include "secrets.h"        //Certs to Access TLS based MQTT Server

bool state=0;
const byte SWITCH_PIN = 0;           // Pin to control the light with
const char *ID = "Example_Switch";   // Name of our device, must be unique
const char *TOPIC = "room/light";    // Topic to subcribe to

WiFiClientSecure espClient;
PubSubClient client1(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Connect to WiFi network
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  Serial.println("TestMQTT");

  pinMode(SWITCH_PIN,INPUT);  // Configure SWITCH_Pin as an input
  digitalWrite(SWITCH_PIN,HIGH);  // enable pull-up resistor (active low)
  delay(100);
  
  setup_wifi(); // Connect To Network
  client1.setServer(mqtt_server, 8883);
  client1.setCallback(callback);
  Serial.print("HEAP: ");  
  Serial.println(ESP.getFreeHeap());
  mqttconnect();
  
  Serial.print("HEAP: "); 
  Serial.println(ESP.getFreeHeap());
}

void mqttconnect()
{
  while (!client1.connected()) {

    Serial.print("Attempting MQTT connection...");
    verifyFingerprint();
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    Serial.print("\nWifi Client ID is : ");
    Serial.print(clientId);
    
    if (client1.connect(clientId.c_str())) {
      Serial.println("connected");
      client1.publish("MYESP8266", "Starting....");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client1.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
       espClient.stop();
        espClient.flush();
        Serial.print("HEAP: ");  
      Serial.println(ESP.getFreeHeap());
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);
}

void verifyFingerprint() {
  if(client1.connected() || espClient.connected()) return;
  
  Serial.print("Checking TLS @ ");
  Serial.print(mqtt_server);
  Serial.print("...");
  Serial.println("");
  espClient.setClientRSACert(new BearSSL::X509List(client_cert), new BearSSL::PrivateKey(client_key));
 // BearSSL::X509List *serverTrustedCA = new BearSSL::X509List(ca_cert);
 espClient.setCACert_P(&ca_cert,sizeof(ca_cert));
 //espClient.setTrustAnchors(serverTrustedCA);
  
 timeClient.begin();
  while(!timeClient.update()) {
  timeClient.forceUpdate();
  } Serial.print("Time:");
  Serial.println(timeClient.getEpochTime());
//  espClient.setX509Time(1600660628);//timeClient.getEpochTime());
  espClient.setX509Time(timeClient.getEpochTime());
  bool mfln = espClient.probeMaxFragmentLength(mqtt_server, 8883, 512);
  
  Serial.printf("MFLN supported: %s\n", mfln ? "yes" : "no");
  if (mfln) {
    espClient.setBufferSizes(512, 512);
  }
  
 if (!espClient.connect(mqtt_server, 8883)) {
    Serial.println("Connection failed. Rebooting.");
    ESP.restart();
  }
  if (espClient.verifyCertChain( mqtt_server.toString().c_str())) {
    Serial.print("Connection secure -> .");
  } else {
    Serial.println("Connection insecure! Rebooting.");
 //   ESP.restart();
  }
  espClient.stop();

  delay(1000);
}

void loop() {
/* if client was disconnected then try to reconnect again */
  if (!client1.connected()) {
    mqttconnect();
  }
  client1.loop();

// if the switch is being pressed
  if(digitalRead(SWITCH_PIN) == 0) 
  {
    state = !state; //toggle state
    if(state == 1) // ON
    {
      client1.publish(TOPIC, "on");
      Serial.println((String)TOPIC + " => on");
    }
    else // OFF
    {
      client1.publish(TOPIC, "off");
      Serial.println((String)TOPIC + " => off");
    }

    while(digitalRead(SWITCH_PIN) == 0) // Wait for switch to be released
    {
      // Let the ESP handle some behind the scenes stuff if it needs to
      yield(); 
      delay(20);
    }
  }
}
