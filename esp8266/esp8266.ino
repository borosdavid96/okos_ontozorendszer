/****************************************
  Könyvtárak
 ****************************************/
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <iostream>
#include <stdlib.h>
/****************************************
   Változók
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
const unsigned long REFRESH_INTERVAL = 10000; // ms
unsigned long lastRefreshTime = 0;
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
int adatszam=7;
bool lekerdezesek[9];
bool ket_eszkoz_e;
bool szelloztetes;
bool volt_e[3];


ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);

/****************************************
   Függvények
 ****************************************/
 void const pub(const char * valtozonev, float valtozo) {

  sprintf(topic, "%s", ""); // Cleans the topic content
  sprintf(topic, "%s%s", "/v2.0/devices/", DEVICE1); ///v2.0/devices/{LABEL_OF_DEVICE}/{LABEL_OF_VARIABLE}
  sprintf(payload, "{\"%s\":", valtozonev); // Adds the variable label
  sprintf(payload, "%s {\"value\": %d", payload, valtozo); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  std::cout << "Publikálva! A payload tartalma:" << payload << std::endl;
  client.publish(topic, payload);
}
void const sub(const char* eszköz, const char* valtozo) {
  sprintf(topicToSubscribe, "%s", ""); // Cleans the content of the char
  sprintf(topicToSubscribe, "%s/%s/%s/lv", "/v2.0/devices", eszköz, valtozo);
  Serial.println(topicToSubscribe);
  client.subscribe(topicToSubscribe); //  /v2.0/devices/{LABEL_DEVICE}/{LABEL_VARIABLE}/lv
}
 void subscribe_all(){
    sub(DEVICE1, "homerseklet");
    sub(DEVICE1, "dht11-hom");
    sub(DEVICE2, "tartomany");
    sub(DEVICE2, "alvasi_ido");
    sub(DEVICE1, "adat");
    sub(DEVICE2, "ket_eszkoz_e");
    sub(DEVICE1, "szelloztetes");
  }
void sleeping() {
  delay(5000); // hogy bevárja az első eszközt
  std::cout << "Sleeping for " << sleepingtime << " minutes" << std::endl;
  ESP.deepSleep(sleepingtime * uS_TO_M);
}
void adat_lekeres_ellenorzes(){
  std::cout << "Adatok lekérése, ha nincs meg minden adat nem indul el a program további része!" << std::endl;
  int i=0;
  for(i=0;i<adatszam;i++){
    while(!lekerdezesek[i]){
      client.loop();// subcribe adatok lekérése
      delay(100);
  }
 }
}

void relays_off(){
  digitalWrite(relay1, HIGH); //a relék fordított logikája miatt kell , így induláskor nem aktiválódik a relé
  digitalWrite(relay2, HIGH);
  }

void callback(char* topic, byte* payload, unsigned int length) {
 char buff [6];
  Serial.print("Üzenet jött [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    buff[i]=((char)payload[i]);
  }
  value=atoi(buff);
  Serial.println();

  if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany ertekeke:" << value << std::endl;
    Tartomany = value;
    lekerdezesek[0]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/dht11-hom/lv")) {
    std::cout << "A lekérdezett dht_homerseklet erteke:" << value << std::endl;
    dht_homerseklet = value;
    lekerdezesek[1]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/alvasi_ido/lv")) {
    std::cout << "A lekérdezett alvasi_ido:" << value << std::endl;
    sleepingtime = value;
    lekerdezesek[2]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/adat/lv")) {
    std::cout << "A lekérdezett Sleep:" << value << std::endl;
    if (value == 1) sleeping();
    lekerdezesek[3]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    homerseklet = value;
    lekerdezesek[4]=true;
  }
    else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ket_eszkoz_e/lv")) {
    std::cout << "A lekérdezett ket_eszkoz_e:" << value << std::endl;
    ket_eszkoz_e = value;
    lekerdezesek[5]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/szelloztetes/lv")) {
    std::cout << "A lekérdezett szelloztetes:" << value << std::endl;
    szelloztetes = value;
    lekerdezesek[6]=true;
  }
}

void reconnect() {
  // addig amíg nincs kapcsolat próbálkozás
  while (!client.connected()) {
    Serial.println("MQTT kapcsolat létrehozása...");

    // csatlakozasi kísérlet
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Csatlakoztatva");
    } else {
      Serial.print("Hiba, kliens státusz=");
      Serial.print(client.state());
      Serial.println(" újrapróbálkozás 2 másodpercen belül");
      // várakozás
      delay(2000);
    }
  }
}



/****************************************
  Main
 ****************************************/

void setup() {
  Serial.begin(115200);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  relays_off();
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  Serial.println();
  Serial.print("Várakozás wifire... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi csatlakoztatva");
  Serial.println("IP cím: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  randomSeed(analogRead(0));
  timer = millis();
  lastRefreshTime = millis();
  reconnect();
  subscribe_all();
  adat_lekeres_ellenorzes();
  
}
void loop() {

  if (!client.connected()) {
    relays_off();
    reconnect();
    subscribe_all();
    // Subscribes for getting the value , számít a sorrend
  
  }
if (ket_eszkoz_e){
  if(szelloztetes){
    if(!volt_e[0]){
    std::cout << "A szellőztetés megy." << std::endl;
    digitalWrite(relay1, LOW);
    volt_e[0]=true;
      }
   
      
    }
  else if(labs(millis() - lastRefreshTime >= REFRESH_INTERVAL)) // triggers the routine every x seconds  //abs helyett labs kellett
  {
    if (homerseklet + Tartomany <= dht_homerseklet) { //ha a lekérdezett kettő érték összegénél kisebb beavatkozás
      volt_e[1]=true;
      digitalWrite(relay1, LOW);
      std::cout << "A hűtés megy." << std::endl;
    }
    else if (volt_e[1] || volt_e[0]){
      digitalWrite(relay1, HIGH);
      std::cout << "Hűtés ki." << std::endl;
      volt_e[0]=false;
      volt_e[1]=false;
    }
    lastRefreshTime = millis();//az időzítő innen számolja az eltelt időt
  }
}
  client.loop();
  delay(500);
}
