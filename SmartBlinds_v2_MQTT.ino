#include <Servo.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#define SERVO_PIN 12 // GPIO 12 is pin D6 on the D1 Mini

/*******************************************************
 Blinds Spacific Settings, 
 Needs to be modified for each set of blinds.
 The "mqttDeviceID" needs to be unique for each set of blinds
 Below are six (6) blinds configurations
 
 *******************************************************/
// !! ONLY 1 OF THE CODE BLOCKS BELOW SHOULD BE UNCOMMENTED. !!
/*
//Right Living Room Blinds - 192.168.20.11
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_living_right";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/living";
const char* subscribeTopic3 = "home/blinds/living/right";
const char* pubTopic = "home/blinds/living/right/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_living_right"; 
//*/

/*
//Left Living Room Blinds - 192.168.20.10
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_living_left";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/living";
const char* subscribeTopic3 = "home/blinds/living/left";
const char* pubTopic = "home/blinds/living/left/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_living_left"; 
/*/

/*
//Right Kitchen Blinds - 192.168.20.7 *****
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_kitchen_right";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/kitchen";
const char* subscribeTopic3 = "home/blinds/kitchen/right";
const char* pubTopic = "home/blinds/kitchen/right/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_kitchen_right"; 
//*/

/*
//Left Kitchen Room Blinds - 192.168.20.9
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_kitchen_left";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/kitchen";
const char* subscribeTopic3 = "home/blinds/kitchen/left";
const char* pubTopic = "home/blinds/kitchen/left/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_kitchen_left"; 
//*/

/*
//Right Dining Room Blinds - 192.168.20.12 ******
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_dining_right";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/dining";
const char* subscribeTopic3 = "home/blinds/dining/right";
const char* pubTopic = "home/blinds/dining/right/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_dining_right"; 
//*/

/*
//Left Dining Room Blinds - 192.168.20.8
// Web Updater Settings 
// Host Name of Device
const char* host = "blinds_dining_left";

// MQTT Settings 
// Topic which listens for commands
const char* subscribeTopic1 = "home/blinds/all";
const char* subscribeTopic2 = "home/blinds/dining";
const char* subscribeTopic3 = "home/blinds/dining/left";
const char* pubTopic = "home/blinds/dining/left/state";

//Unique device ID 
const char* mqttDeviceID = "blinds_dining_left"; 
//*/

/* WIFI Settings */
// Name of wifi network
const char* ssid = "JarvisIoT";

// Password to wifi network
const char* password = "mmier947$IOT"; 

// Path to access firmware update page (Not Neccessary to change)
const char* update_path = "/firmware";

// Username to access the web update page
const char* update_username = "admin";

// Password to access the web update page
const char* update_password = "admin";

//MQTT Server IP Address
//const char* server = "192.168.86.9";
//IPAddress server = IPAddress(192, 168, 86, 9);
const char* mqtt_server = "192.168.86.9";
const char* mqtt_username = "haBroker"; // MQTT username
const char* mqtt_password = "mmier947"; // MQTT password
int currentInterval;
const int PUBLISH_INTERVAL = 300;

// Servo Control Variable
const int MIN_PULSE = 500;
const int MAX_PULSE = 2500;
const int MIN_ANGLE = 0;
const int MAX_ANGLE = 180;
const int HALF_ANGLE = 90;
char* blinds_state;
Servo myservo;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiClient net;
PubSubClient client(net);

void setup() 
{
  currentInterval = 0;
  blinds_state = "UNKNOWN";
  
  delay(100);
  
  // Setup WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Connect Servo
  myservo.attach(SERVO_PIN);

  // Setup Update Server
  MDNS.begin(host);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() 
{
  // Increment interval
  currentInterval = currentInterval + 1;

  // Attempt to connect
  while (!client.connected()) 
  {
    if (client.connect(mqttDeviceID,mqtt_username,mqtt_password))
    {
     //once connected to MQTT broker, subscribe command if any
      client.subscribe(subscribeTopic1);
      client.subscribe(subscribeTopic2);
      client.subscribe(subscribeTopic3);
    } else {
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
  
  // Update the publication topic every X times the loop has run
  if (currentInterval > PUBLISH_INTERVAL) {
    currentInterval = 0;
    char msgBuffer[20];
    if (client.publish(pubTopic, blinds_state)) {
      //Serial.println("Front Door State sent!");
    } else {
      //Serial.println("Front Door State failed to send. Reconnecting to MQTT Broker and trying again");
      client.connect(mqttDeviceID, mqtt_username, mqtt_password);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(pubTopic, blinds_state);
    }
  }
  
  client.loop();
  
  delay(10);

  httpServer.handleClient();
}

boolean isValidNumber(String str)
{
   boolean isNum=false;
   if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false;

   for(byte i=1;i<str.length();i++)
   {
       if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;
   }
   return true;
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String msgString = "";
  for (int i=0;i<length;i++) {
    msgString = msgString + String((char)payload[i]);
  }
  if(!myservo.attached()) {
    myservo.attach(SERVO_PIN);
  }
  if (msgString == "UP")
  {
    blinds_state = "UP";
    myservo.write(MIN_ANGLE);               
    delay(1000);  
  }
  else if (msgString == "DOWN")
  {
    blinds_state = "DOWN";
    myservo.write(MAX_ANGLE);              
    delay(1000);  
  }
  else if (msgString == "HALF")
  {
    blinds_state = "HALF";
    myservo.write(HALF_ANGLE);               
    delay(1000);  
  }
  else if (isValidNumber(msgString))
  {
    blinds_state = "CUSTOM";
    msgString = msgString.toInt() < MIN_ANGLE ? (String)MIN_ANGLE : msgString;
    msgString = msgString.toInt() > MAX_ANGLE ? (String)MAX_ANGLE : msgString;
    myservo.write(msgString.toInt());               
    delay(1000); 
  }
  
}
