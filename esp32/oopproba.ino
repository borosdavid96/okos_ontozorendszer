
/****************************************
           Könyvtárak
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <iostream>
#include "values.h"
#include "szenzorok.h"

/****************************************
       Változók
 ****************************************/

unsigned long lastRefreshTime = 0;
unsigned long lastRefreshTime1 = 0;

Szenzorok proba;
const int REFRESH_INTERVAL = 10000; // Update rate in milliseconds
const int REFRESH_INTERVAL1 = 33000; // Update rate in milliseconds

int kapcs = 0;
int value = 0;

/****************************************
        Függvények
 ****************************************/

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

  if (!strcmp(topic, "/v2.0/devices/esp1/pumpa/lv")) {
    std::cout << "A lekérdezett pumpa értéke:" << value << std::endl;
    proba.beolvas.pumpa = value;
    proba.lekerdezesek[0]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    proba.beolvas.homerseklet = value;
    proba.lekerdezesek[1]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany:" << value << std::endl;
    proba.beolvas.Tartomany = value;
    proba.lekerdezesek[2]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/paratartalom/lv")) {
    std::cout << "A lekérdezett parasítási határérték:" << value << std::endl;
    proba.beolvas.paratartalom = value;
    proba.lekerdezesek[3]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ontozesidotartam/lv")) {
    std::cout << "A lekérdezett ontozesidotartam:" << value << std::endl;
    proba.beolvas.ontozesidotartam = value;
    proba.lekerdezesek[4]=true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/sleepingtime/lv")) {
    std::cout << "A lekérdezett sleep_time:" << value << std::endl;
    proba.beolvas.sleep_time = value;
    proba.lekerdezesek[5]=true;
  }

}
void print_wakeup_reason() {

  esp_sleep_wakeup_cause_t wakeupreason;
  wakeupreason = esp_sleep_get_wakeup_cause();

  switch (wakeupreason)
  {
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP"); break;
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    
    default : Serial.printf("Wakeup wasnt caused by deep sleep: %d\n", wakeupreason); break;
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
  proba.relays_off();

  ubidots.setDebug(false);  // uncomment this to make debug messages available
  print_wakeup_reason();
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  proba.subscribe_all();
  dht.begin();
  tempSensor.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);//pullup:inputnál nem szükséges ellenállás bekötése ahogy az esp8266-nál, ez a vízszint kapcsolóhoz kell
  lastRefreshTime = millis();
  
  std::cout << "Adatok lekérése." << std::endl;
  while(!proba.lekerdezesek[5]){
  ubidots.loop();// subcribe adatok lekérése
  delay(200);
    }
  
  lastRefreshTime = millis();
  lastRefreshTime1 = millis();
}

void loop()
{

 proba.ujracsatlakozas();

  if (labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL) // abszolútértéke a kettő különbségének amíg kisebb mint a kijelölt időtartomány nem fog futni , triggers the routine every x seconds
  {
    proba.update();
    proba.print();
    proba.vezerles();
    proba.inaktiv();

    lastRefreshTime = millis(); //elmnetjük az időt hogy innen számoljuk
  }

if (labs(millis() - lastRefreshTime1) >= REFRESH_INTERVAL1) // triggers the routine every x seconds
  {
    proba.publikalas();  
    lastRefreshTime1 = millis(); //elmnetjük az időt hogy innen számoljuk
  }
  
  proba.ontoz_ki();
  ubidots.loop();// subcribe adatok lekérése
  delay (500);
}
