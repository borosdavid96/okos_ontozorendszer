
/****************************************
           Könyvtárak
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <iostream>
#include "values.h"
#include "szenzorok.h"

#define uS_TO_M 60000000  //Conversion micro seconds to seconds

/****************************************
       Változók
 ****************************************/
bool pub = 1; //publikáció kikapcsolása 0 , 1 publikálás
int pumpa;
int homerseklet; //weblapos meghatározott hőmérséklet
int paratartalom;
float dht_adat[2];
int vizertek;
unsigned long lastRefreshTime = 0;
bool first_run = true;
bool beavatkozas[4];
int uresfutas_szamlalo = 0;
int sleep_time = 30;
int alvas = 0;
int Tartomany = 0;
int ontozesidotartam = 5;
int kapcs = 0;
int value = 0;
/****************************************
       Konstansok
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-l9otOKSgN1ghRt6IR5EIVsfHqlOVIv";            // Put here your Ubidots TOKEN
const char *WIFI_SSID = "BorosD";           // Put here your Wi-Fi SSID
const char *WIFI_PASS = "borosdavid";       // Put here your Wi-Fi password
const char *DEVICE2 = "b4e62d04cda2"; // Assig the device label
const char *DEVICE1 = "esp1"; // Assig the device label
const int REFRESH_INTERVAL = 25000; // Update rate in milliseconds

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
        Függvények
 ****************************************/
void sleep () {
  //Set sleep timer to x seconds
  delay(1000);
  alvas = 1;
  ubidots.add("adat", alvas); //1 küldése hogy a második mikrokontroller aludjon
  ubidots.publish("esp1");
  delay(5000);
  alvas = 0;
  ubidots.add("adat", alvas); //0 küldése utána ha esetleg előbb ébredne fel a második ne aludjon az újbóli lekérdezése miatt
  ubidots.publish("esp1");
  esp_sleep_enable_timer_wakeup(sleep_time * uS_TO_M);
  Serial.println(" ESP32 going to sleep for " + String(sleep_time) +
                 " Minutes");
  esp_deep_sleep_start();  //Go to sleep now
}

void subscribe_all() {

  ubidots.subscribeLastValue(DEVICE1, "paratartalom"); // Insert the device and variable's Labels, respectively
  ubidots.subscribeLastValue(DEVICE1, "pumpa");
  ubidots.subscribeLastValue(DEVICE1, "homerseklet");
  ubidots.subscribeLastValue(DEVICE2, "sleepingtime");
  ubidots.subscribeLastValue(DEVICE2, "tartomany");
  ubidots.subscribeLastValue(DEVICE2, "ontozesidotartam");
}
void relays_off (){
  digitalWrite(relay1, HIGH); //fordított logika miatt magasra állítás (tehát kikapcsolt állapotban tartás
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  }

void ontoz() {
  digitalWrite(relay1, LOW);
  Serial.println("Ontozes inditasa.");
  delay(ontozesidotartam * 1000); //lekérdezett idotartam
  digitalWrite(relay1, HIGH);
}

void vilagitas() {
  digitalWrite(relay2, LOW);
  Serial.println("Lampa inditasa.");
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
  Serial.print("]");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  std::cout << topic << std::endl;
/*
  if (!strcmp(topic, "/v2.0/devices/esp1/pumpa/lv")) {
    std::cout << "A lekérdezett pumpa értéke:" << value << std::endl;
    pumpa = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    homerseklet = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany:" << value << std::endl;
    Tartomany = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/paratartalom/lv")) {
    std::cout << "A lekérdezett parasítási határérték:" << value << std::endl;
    paratartalom = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ontozesidotartam/lv")) {
    std::cout << "A lekérdezett ontozesidotartam:" << value << std::endl;
    ontozesidotartam = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/sleepingtime/lv")) {
    std::cout << "A lekérdezett sleep_time:" << value << std::endl;
    sleep_time = value;
  }
*/
  switch(topic) {
    case "/v2.0/devices/esp1/pumpa/lv":
     std::cout<<"A lekérdezett pumpa értéke:"<<value<<std::endl;
     pumpa=value;
     break;
    case"/v2.0/devices/esp1/homerseklet/lv":
     std::cout<<"A lekérdezett alsó hőmérsékleti hatar:"<<value<<std::endl;
     homerseklet=value;
     break;
      case "/v2.0/devices/b4e62d04cda2/tartomany/lv":
     std::cout<<"A lekérdezett Tartomany:"<<value<<std::endl;
     Tartomany=value;
     break;
      case "/v2.0/devices/esp1/paratartalom/lv":
      std::cout<<"A lekérdezett parasítási határérték:"<<value<<std::endl;
     paratartalom=value;
     break;
      case "/v2.0/devices/b4e62d04cda2/ontozesidotartam/lv":
      std::cout<<"A lekérdezett ontozesidotartam:"<<value<<std::endl;
     ontozesidotartam=value;
     break;
      case "/v2.0/devices/b4e62d04cda2/sleepingtime/lv":
      std::cout<<"A lekérdezett sleep_time:"<<value<<std::endl;
     sleep_time=value;
     break;
    default:
     std::cout<<"A jött adat nem szerepel a vártak között."<<std::endl;
    }
  

}

void print_wakeup_reason() {

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP"); break;
    default : Serial.printf("Wakeup wasnt caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
/****************************************
          Main
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(relay1, OUTPUT); //relay
  pinMode(relay2, OUTPUT); //relay
  pinMode(relay3, OUTPUT); //relay
  pinMode(relay4, OUTPUT); //relay
  relays_off();

  ubidots.setDebug(false);  // uncomment this to make debug messages available
  print_wakeup_reason();
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  subscribe_all();
  dht.begin();
  tempSensor.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);//pullup:inputnál nem szükséges ellenállás bekötése ahogy az esp8266-nál, ez a vízszint kapcsolóhoz kell

  lastRefreshTime = millis();
}

void loop()
{
  // main code here, to run repeatedly:
  if (first_run)
  {
    Serial.println("Elso futas, 3ms várakozás"); //szenzorok inicializálására időt kell hagyni induláskor
    delay (3000);
    first_run = false;
  }

  if (!ubidots.connected()) //ujracsatlakozas
  {
    relays_off();
    ubidots.reconnect();
    subscribe_all();
  }


  if (labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL) // abszolútértéke a kettő különbségének amíg kisebb mint a kijelölt időtartomány nem fog futni , triggers the routine every x seconds
  {

    //soil-be
    int soilmoisture_value = foldnedv_olvas();

    //dht11-be
    dht_beolvas(dht_adat);

    //onewire-be
    float fold_homerseklet = fold_hom();//fold hom olvasása

    //vizszint-be
    vizertek = vizszint_olvas();

    //soil-kiir
    soilmoisture_value = szazalekosit(soilmoisture_value);
    soilmoisture_print(soilmoisture_value);

    //dht11-kiir
    dht_kiir(dht_adat);

    //onewire-kiir
    fold_kiir(fold_homerseklet);

    //vizszint-kiir
    vizszint_kiir(vizertek);

    //vezérlés majd ide
    if (soilmoisture_value < pumpa ) { //x másodpercenként öntöz egyszer x ideig
      beavatkozas[0] = true;
      ontoz();
    }
    else {
      beavatkozas[0] = false;
    }
    if (fold_homerseklet < homerseklet) { //addig világít amíg x másodperces mintavételezéssel jó nem lesz
      beavatkozas[1] = true;
      vilagitas();
    }
    else {
      beavatkozas[1] = false;
      digitalWrite(relay2, HIGH);
    }
    if (dht_adat[0] < paratartalom) {
      beavatkozas[2] = true;
      digitalWrite(relay3, LOW);
      std::cout << "Parasító megy" << std::endl;
    }
    else {
      beavatkozas[2] = false;
      digitalWrite(relay3, HIGH);
    }
    if (homerseklet + Tartomany < (int)dht_adat[1]) {
      beavatkozas[3] = true;
      //digitalWrite(relay4,LOW); //akkor kell csak ha a második nincs
      std::cout << "Venti megy" << std::endl;
    }
    else {
      beavatkozas[3] = false;
      //digitalWrite(relay4,HIGH); //akkor kell csak ha a második eszköz nincs
    }

    if (pub == 1) { //publikalas be-ki

      // Insert your variable Labels and the value to be sent
      ubidots.add("soil", soilmoisture_value); //(változónév a felhőben,változónév lokálisan)
      ubidots.add("dht11-parat", dht_adat[0]);
      ubidots.add("dht11-hom", dht_adat[1]);
      ubidots.publish(DEVICE1);//az eszköz neve ahová publikálni szeretnénk
      delay(1000); // egy másodperc alatt maximum 4et tudunk publikálni ezért utána időt kell hagyni különben csak néha vagy nem lesz publikálva ami ezen túl csúszik
      ubidots.add("foldhom", fold_homerseklet);
      ubidots.add("vizszint", vizertek);
      ubidots.publish(DEVICE1);
      std::cout << "Publikalas vegrehajtva!" << std::endl;

    }
    if (beavatkozas[0] + beavatkozas[1] + beavatkozas[2] + beavatkozas[3] < 1) { // ha nem kellett beavatkozás számláló++
      uresfutas_szamlalo += 1;
      std::cout << "Üres futás " << uresfutas_szamlalo << " alkalommal" << std::endl;
    }
    else {
      uresfutas_szamlalo = 0;
    }

    lastRefreshTime = millis(); //elmnetjük az időt hogy innen számoljuk
  }
  ubidots.loop();// subcribe adatok lekérése

  if (uresfutas_szamlalo > 1) { //alvas ha x szer nincs semmi beavatkozás
    sleep();
  }

  delay (1000);
}
