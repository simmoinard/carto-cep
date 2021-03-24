/* This example shows how to use MQTT on the main dev boards on the market
  HOW TO USE:
  under connect method, add your subscribe channels.
  under messageReceived (callback method) add actions to be done when a msg is received.
  to publish, call client.publish(topic,msg)
  in loop take care of using non-blocking method or it will corrupt.
  Alberto Perro & DG - Officine Innesto 2019
*/
#define BROKER_IP    "192.168.4.1"
#define DEV_NAME     "mqttdevice_4"
#define MQTT_USER    "moinards"
#define MQTT_PW      "kbrf-412"
#define TOPIC      "/sensor_2"
#define BROKER_PORT 1883
const char ssid[] = "raspberry_bricolab";
const char pass[] = "pyramide85R$";

#include <MQTT.h>
#include <WiFiNINA.h>

WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

const byte TRIGGER_PIN = 2; // Broche TRIGGER
const byte ECHO_PIN = 3;    // Broche ECHO
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s
const float SOUND_SPEED = 340.0 / 1000;
int id = 0;

void setup() {
 Serial.begin(115200);
 analogReference(AR_INTERNAL1V0);
 WiFi.begin(ssid, pass);
 client.begin(BROKER_IP, BROKER_PORT, net);
 client.onMessage(messageReceived);
 connect();

 pinMode(TRIGGER_PIN, OUTPUT);
 digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
 pinMode(ECHO_PIN, INPUT);
}

void loop() {
  float distance = hcsr();
  Serial.println(distance);
  if (distance < 600 && distance > 50) {
    delay(10);
    float distance_2 = hcsr();
    Serial.println(distance);
    if (distance_2 < 600 && distance_2 > 50) {
      digitalWrite(LED_BUILTIN, HIGH); 
      client.loop();
      if (!client.connected()) {
        connect();
        Serial.println("Trying to connect in loop");
      }
      Serial.println("connected in loop.");
      String msg =  String(id);  
      client.publish(TOPIC, msg); //PUBLISH TO TOPIC /hello MSG world
      Serial.println("msg sent");
      id++;
      delay(250);
      digitalWrite(LED_BUILTIN, LOW); 

    }
  }
}

void connect() {
 Serial.print("checking wifi...");
 while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   delay(1000);
 }
 Serial.print("Wifi OK.");
 Serial.print("\nconnecting...");
 while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
   Serial.print(".");
   delay(1000);
 }
 Serial.println("\nconnected!");
 client.subscribe(TOPIC); //SUBSCRIBE TO TOPIC
}

void messageReceived(String &topic, String &payload) {
 Serial.println("incoming: " + topic + " - " + payload);
 if (topic == TOPIC) {
   if (payload == "open") {
     Serial.println("open");
     digitalWrite(LED_BUILTIN, HIGH); 
   } else if (payload == "closed") {
     Serial.println("closed");
     digitalWrite(LED_BUILTIN, LOW); 
   }
 }
}

float hcsr(){
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);
  float distance_mm = measure / 2.0 * SOUND_SPEED;
  return distance_mm;
}
