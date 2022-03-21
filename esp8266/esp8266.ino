/****************************************
   Include Libraries
 ****************************************/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <iostream>
#include <stdlib.h>
/****************************************
   Define Constants
 ****************************************/
#define relay1 5
#define relay2 16
#define uS_TO_M 60000000  //Conversion micro seconds to seconds
#define WIFISSID "BorosD" // Put your WifiSSID here
#define PASSWORD "borosdavid" // Put your wifi password here
#define TOKEN "BBFF-l9otOKSgN1ghRt6IR5EIVsfHqlOVIv" // Put your Ubidots' TOKEN
#define DEVICE2 "b4e62d04cda2" // Assig the device label
#define DEVICE1 "esp1" // Assig the device label
#define MQTT_CLIENT_NAME "Ubidots" // MQTT client Name
static const unsigned long REFRESH_INTERVAL = 25000; // ms
static unsigned long lastRefreshTime = 0;
char mqttBroker[] = "things.ubidots.com";
char payload[700];
char topic[150];
int Tartomany;
int dht_homerseklet;
int homerseklet;
static unsigned long timer = 0;
int sleep = 0;
int sleepingtime = 10;
int value = 0; 
char topicToSubscribe[200];

/****************************************
   Initializate constructors for objects
 ****************************************/

ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);

/****************************************
   Auxiliar Functions
 ****************************************/
void sleeping() {
  delay(5000); // hogy bevárja az első eszközt
  std::cout << "Sleeping for " << sleepingtime << " minutes" << std::endl;
  ESP.deepSleep(sleepingtime * uS_TO_M);
}

int btof(byte * payload, unsigned int length) {
  char * buff = (char *) malloc(sizeof(char) * 10);
  for (int i = 0; i < length; i++) {
    buff[i] = payload[i];
  }
  int value = atoi(buff);
  free(buff);
  return value;
}
void relays_off(){
  digitalWrite(relay1, HIGH); //a relék fordított logikája miatt kell , így induláskor nem aktiválódik a relé
  digitalWrite(relay2, HIGH);
  }

void callback(char* topic, byte* payload, unsigned int length) {
  value = btof(payload, length);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany ertekeke:" << value << std::endl;
    Tartomany = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/dht11-hom/lv")) {
    std::cout << "A lekérdezett dht_homerseklet erteke:" << value << std::endl;
    dht_homerseklet = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/sleepingtime/lv")) {
    std::cout << "A lekérdezett Sleep_time:" << value << std::endl;
    sleepingtime = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/adat/lv")) {
    std::cout << "A lekérdezett Sleep:" << value << std::endl;
    if (value == 1) sleeping();
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    homerseklet = value;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("Failure, client state=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

int const pub(const char * valtozonev, float valtozo) {

  sprintf(topic, "%s", ""); // Cleans the topic content
  sprintf(topic, "%s%s", "/v2.0/devices/", DEVICE1); ///v2.0/devices/{LABEL_OF_DEVICE}/{LABEL_OF_VARIABLE}
  sprintf(payload, "{\"%s\":", valtozonev); // Adds the variable label
  sprintf(payload, "%s {\"value\": %d", payload, valtozo); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  std::cout << "Publikálva! A payload tartalma:" << payload << std::endl;
  client.publish(topic, payload);
  return 1;
}
int const sub(const char* eszköz, const char* valtozo) {
  sprintf(topicToSubscribe, "%s", ""); // Cleans the content of the char
  sprintf(topicToSubscribe, "%s/%s/%s/lv", "/v2.0/devices", eszköz, valtozo);
  Serial.println(topicToSubscribe);
  client.subscribe(topicToSubscribe); //  /v2.0/devices/{LABEL_DEVICE}/{LABEL_VARIABLE}/lv
  return 1;
}

/****************************************
   Main Functions
 ****************************************/

void setup() {
  Serial.begin(115200);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  relays_off();
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  randomSeed(analogRead(0));
  timer = millis();
  lastRefreshTime = millis();
}

void loop() {

  if (!client.connected()) {
    relays_off();
    reconnect();

    // Subscribes for getting the value , számít a sorrend
    sub(DEVICE1, "homerseklet");
    sub(DEVICE1, "dht11-hom");
    sub(DEVICE2, "tartomany");
    sub(DEVICE2, "sleepingtime");
    sub(DEVICE1, "adat");

  }

  if (labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL) // triggers the routine every x seconds  //abs helyett labs kellett
  {
    // pub(); //publikálás
    if (homerseklet + Tartomany < dht_homerseklet) { //ha a lekérdezett kettő érték összegénél kisebb beavatkozás
      digitalWrite(relay1, LOW);
      std::cout << "Venti megy" << std::endl;
    }
    else {
      digitalWrite(relay1, HIGH);
    }
    lastRefreshTime = millis();//az időzítő innen számolja az eltelt időt
  }
  client.loop();
  delay(1000);
}
