/****************************************
 * Include Libraries
 ****************************************/
 
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <iostream>
#include <stdlib.h>  
/****************************************
 * Define Constants
 ****************************************/

#define WIFISSID "BorosD" // Put your WifiSSID here
#define PASSWORD "borosdavid" // Put your wifi password here
#define TOKEN "BBFF-l9otOKSgN1ghRt6IR5EIVsfHqlOVIv" // Put your Ubidots' TOKEN
#define VARIABLE_LABEL_SUB "kapcs" // Assing the variable label
#define DEVICE_LABEL_SUB "esp1" // Assig the device label
#define VARIABLE_LABEL_PUB "kapcs" // Assing the variable label
#define DEVICE_LABEL_PUB "esp1" // Assig the device label
#define MQTT_CLIENT_NAME "Ubidots" // MQTT client Name
static const unsigned long REFRESH_INTERVAL = 10000; // ms
static unsigned long lastRefreshTime = 0;
char mqttBroker[] = "things.ubidots.com";
char payload[700];
char topic[150];
static unsigned long timer=0;
// Space to store values to send
char str_temp[6];
char str_lat[6];
char str_lng[6];

int value=1; //a relének kell külön változó és először nemet kell kapnia mert különben beindul indításkor


/****************************************
 * Initializate constructors for objects
 ****************************************/

ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);


/****************************************
 * Auxiliar Functions
 ****************************************/
 int btof(byte * payload, unsigned int length) {
  char * demo = (char *) malloc(sizeof(char) * 10);
  for (int i = 0; i < length; i++) {
    demo[i] = payload[i];
  }
  value = atoi(demo);
  free(demo);
  return value;
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  value = btof(payload, length);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(value==1){
    digitalWrite(5,LOW); //relé
    digitalWrite(16, LOW); //vilagitas proba
  }
   if(value==0){
    digitalWrite(5,HIGH); //relé
     digitalWrite(16, HIGH); //vilagitas proba
  }
} 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN,"")) {
      Serial.println("connected");
    } else {
      Serial.print("Hiba, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

  int pub(char * valtozonev,float valtozo) {
 
    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "/v2.0/devices/", DEVICE_LABEL_PUB); ///v2.0/devices/{LABEL_OF_DEVICE}/{LABEL_OF_VARIABLE}
    sprintf(payload, "{\"%s\":", valtozonev); // Adds the variable label   
    sprintf(payload, "%s {\"value\": %d", payload, valtozo); // Adds the value
    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
    std::cout<<"Publikálva! A payload tartalma:"<<payload<<std::endl;
   client.publish(topic, payload);
   return 0;
  }

/****************************************
 * Main Functions
 ****************************************/
 
void setup() {
    Serial.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5,HIGH); //a relék fordított logikája miatt kell , így induláskor nem aktiválódik a relé
     pinMode(16, OUTPUT);
     digitalWrite(16,LOW);
    WiFiMulti.addAP(WIFISSID, PASSWORD);
    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
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
    lastRefreshTime=millis();
}

void loop() {

    if (!client.connected()) {
      reconnect();

      // Subscribes for getting the value of the control variable in the temperature-box device
      char topicToSubscribe[200];
      sprintf(topicToSubscribe, "%s", ""); // Cleans the content of the char
      sprintf(topicToSubscribe, "%s/%s/%s/lv","/v2.0/devices",DEVICE_LABEL_SUB, VARIABLE_LABEL_SUB);
      Serial.println(topicToSubscribe);
      client.subscribe(topicToSubscribe); //v1.6/devices/{LABEL_DEVICE}/{LABEL_VARIABLE}/lv
     //sprintf(topicToSubscribe, "%s/%s/%s/lv","/v2.0/devices","esp1", "lampa_kapcs"); //többszörös feliratkozás
     // Serial.println(topicToSubscribe);
     // client.subscribe(topicToSubscribe); //v2.0/devices/{LABEL_DEVICE}/{LABEL_VARIABLE}/lv
    }
    
    // Values to send
    if(labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL)// triggers the routine every x seconds  //abs helyett labs kellett
  {
    // pub(); //publikálás
    
   // std::cout<<"Timer pubban:"<<millis()<<std::endl;
   // std::cout<<"last ref:"<<lastRefreshTime<<std::endl;
   lastRefreshTime = millis();//az időzítő innen számolja az eltelt időt
  }
    client.loop();
    delay(1000);
}
