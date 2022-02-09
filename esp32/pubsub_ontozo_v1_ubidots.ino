
/****************************************
           Könyvtárak
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <iostream>
#include "values.h"
#include "szenzorok.h"
#define uS_TO_S 1000000  //Conversion micro seconds to seconds
#define TIME_TO_SLEEP  30        // sleeptime (in seconds)

#define relay1 5
#define relay2 18

int elozo_adat=-1;
bool pub=1; //publikáció kikapcsolása 0 , 1 publikálás
int futas=0;
bool pumpa=0;
int lampa=0;
int pumpa_kapcs;
int lampa_kapcs;
float dht_adat [2];

bool first_run=true;

/****************************************
       Konstansok
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-l9otOKSgN1ghRt6IR5EIVsfHqlOVIv";            // Put here your Ubidots TOKEN
const char *WIFI_SSID = "BorosD";                // Put here your Wi-Fi SSID
const char *WIFI_PASS = "borosdavid";                // Put here your Wi-Fi password
const char *PUBLISH_DEVICE_LABEL = "esp1";     // Put here your Device label to which data  will be published
const char *PUBLISH_VARIABLE_LABEL = "kapcs";   // Put here your Variable label to which data  will be published
const char *SUBSCRIBE_DEVICE_LABEL = "esp1";   // Replace with the device label to subscribe to
const char *SUBSCRIBE_VARIABLE_LABEL = "kapcs"; // Replace with the variable label to subscribe to
const int REFRESH_INTERVAL = 25000; // Update rate in millisecondsx
int Kuldott=0;   
static unsigned long lastRefreshTime = 0;
unsigned long lastR = 0;
unsigned long lastreshTime = 0;

int kapcs=0;
int value=0;
Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
        Függvények
 ****************************************/
void sleep (){
   //Set sleep timer to x seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S);
  Serial.println(" ESP32 going to sleep for " + String(TIME_TO_SLEEP) +
  " Seconds");
  esp_deep_sleep_start();  //Go to sleep now
  }
  
void ontoz(){
  digitalWrite(relay1,LOW);
  Serial.println("Onzozes inditasa.");
  delay(10000);
  digitalWrite(relay1,HIGH);
  pumpa_kapcs=4;
     
  }

void vilagitas(){
   digitalWrite(relay2,LOW);
   Serial.println("Lampa inditasa.");
   delay(10000);
   digitalWrite(relay2,HIGH);
   lampa_kapcs=6;
     
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
 
void callback(char *topic, byte *payload, unsigned int length)
{
  value = btof(payload, length);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(value==1){
   kapcs=1;
  }
  if (value==0){
   kapcs=0;
  }
 if(value==3){
  pumpa_kapcs=3;
  pumpa=0;
 }
 if(value==4){
  pumpa_kapcs=4;
  pumpa=1;
 }
 if(value==5){
  lampa_kapcs=5;
  lampa=0;
 }
 if(value==6){
  lampa_kapcs=6;
  lampa=1;
 }
  /* Kapott[0] = payload[0]; 
   Kapott[1] = payload[1];
   Kapott[2] = '\0';
  */
}
/*
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
 */
/****************************************
          Main 
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(relay1, OUTPUT); //relay
  pinMode(relay2, OUTPUT); //relay
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  //print_wakeup_reason();
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, SUBSCRIBE_VARIABLE_LABEL); // Insert the device and variable's Labels, respectively
  ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, "pumpa_kapcs");
  ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, "lampa_kapcs");

   dht.begin();
   tempSensor.begin();
   pinMode(BUTTON_PIN, INPUT_PULLUP);//pullup:inputnál nem szükséges ellenállás bekötése ahogy az esp8266-nál

  lastRefreshTime= millis();
}

void loop()
{
  // put your main code here, to run repeatedly:
 if (first_run)
  {
    Serial.println("Elso futas, 3ms várakozás");
    delay (3000);
    first_run=false;
  }

 if (!ubidots.connected()) //ujracsatlakozas
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, SUBSCRIBE_VARIABLE_LABEL); // Insert the device and variable's Labels, respectively
    ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, "pumpa_kapcs");
    ubidots.subscribeLastValue(SUBSCRIBE_DEVICE_LABEL, "lampa_kapcs");
  }


 
  if(labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL) // abszolútértéke a kettő különbségének amíg kisebb mint a kijelölt időtartomány nem fog futni , triggers the routine every x seconds
  {

 //soil-be
 int soilmoisture_value=foldnedv_olvas();

  //dht11-be
  dht_beolvas(dht_adat);

  //onewire-be
  float fold = fold_hom();//fold hom olvasása

  //vizszint-be
  int vizertek=vizszint_olvas();

   //soil-kiir
 soilmoisture_value = szazalekosit(soilmoisture_value);
 soilmoisture_print(soilmoisture_value);

  //dht11-kiir
  dht_kiir(dht_adat);
 
  //onewire-kiir
  fold_kiir(fold);

  //vizszint-kiir
  vizszint_kiir(vizertek); 
  
   //vezérlés majd ide
  if(soilmoisture_value<29 || pumpa_kapcs==3 ){
    ontoz();
    }
   if(fold<26){
    lampa_kapcs=5;
      }
   else{
      lampa_kapcs=6;
      }
   if(dht_adat[0]<70){
      kapcs=1;
    }
   else{
      kapcs=0;
    }
  if (pub==1){ //publikalas be-ki
    
     // Insert your variable Labels and the value to be sent
    ubidots.add("soil",soilmoisture_value);
    ubidots.add("dht11-parat",dht_adat[0]);
    ubidots.add("dht11-hom",dht_adat[1]);
    ubidots.add("foldhom", fold);
    ubidots.add("vizszint",vizertek);
    ubidots.add(PUBLISH_VARIABLE_LABEL,kapcs);
    ubidots.publish(PUBLISH_DEVICE_LABEL);
    ubidots.add("pumpa_kapcs",pumpa_kapcs);
    ubidots.add("lampa_kapcs",lampa_kapcs); //uj sorba kellett írni mert nem lehet csak 6 darab egymás után, kellenek ezek apublikálások mert ha gombot nyomnak bármelyikre utána vissza kell állítni 0ra
    ubidots.publish(PUBLISH_DEVICE_LABEL);
    std::cout<<"Publikalas vegrehajtva!"<<std::endl;
    
    }
    lastRefreshTime = millis(); //elmnetjük az időt hogy innen számoljuk 
    futas++;
 }
  ubidots.loop();// subcribe adatok lekérése
  
  
 if(lampa_kapcs==6){
      vilagitas();
      }
 
 
  /*if(futas>2){
  sleep();
  }
  */
  delay (1000);
  
}
