
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


/****************************************
        Függvények
 ****************************************/
void callback(char *topic, byte *payload, unsigned int length)
{
  int value = 0;
  char buff [4];
  std::cout << "Üzenet jött a [" << topic << "] topikról " << std::endl;
  for (int i = 0; i < length; i++) {
    buff[i] = ((char)payload[i]);
  }
  value = atoi(buff);
  proba.uzenetek_beolvasasa(topic, value);
}
void print_wakeup_reason() {

  esp_sleep_wakeup_cause_t wakeupreason;
  wakeupreason = esp_sleep_get_wakeup_cause();

  switch (wakeupreason)
  {
    case ESP_SLEEP_WAKEUP_TIMER : std::cout << "Felébredés timer miatt" << std::endl; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : std::cout << "Felébredés érintés miatt" << std::endl; break;
    case ESP_SLEEP_WAKEUP_ULP : std::cout << "Felébredés ULP miatt" << std::endl; break;
    case ESP_SLEEP_WAKEUP_EXT0 : std::cout << "Felébredés külső jel miatt: RTC_IO" << std::endl; break;
    case ESP_SLEEP_WAKEUP_EXT1 : std::cout << "Felébredés külső jel miatt: RTC_CNTL" << std::endl; break;
    default : Serial.printf("A felébredés nem deepsleep miatt: %d\n", wakeupreason); break;
  }
}
/****************************************
          Main
 ****************************************/

void setup()
{
  Serial.begin(115200);

  pinMode(relay1, OUTPUT); //relé
  pinMode(relay2, OUTPUT); //relé
  pinMode(relay3, OUTPUT); //relé
  pinMode(relay4, OUTPUT); //relé
  proba.relek_ki();

  ubidots.setDebug(false);  //aktív állapotban debug üzeneteket ad
  print_wakeup_reason();
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  proba.subscribe_all();
  dht.begin();
  tempSensor.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);//pullup:inputnál nem szükséges ellenállás bekötése ahogy az esp8266-nál, ez a vízszint kapcsolóhoz kell

  proba.adat_lekeres_ellenorzes();
  std::cout << "Adatlekérés sikeres." << std::endl;
  lastRefreshTime = millis();
  lastRefreshTime1 = millis();
}

void loop()
{

  proba.ujracsatlakozas();

  if (proba.get_szelloztetes()) {
    proba.uresfutas_nullazasa();
    proba.relek_ki_kiveve();
    if (!proba.get_ket_eszkoz_e()) {
      digitalWrite(relay4, LOW);
    }

    if (labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL * 6) // abszolútértéke a kettő különbségének amíg kisebb mint a kijelölt időtartomány nem fog futni , triggers the routine every x seconds
    {
      proba.update();
      proba.print();
      proba.ontoz();
      std::cout << "Szellőztetés megy!" << std::endl;

      lastRefreshTime = millis(); //elmnetjük az időt hogy innen számoljuk
    }
    if (labs(millis() - lastRefreshTime1) >= REFRESH_INTERVAL1 * 2) // triggers the routine every x seconds
    {
      proba.publikalas();
      lastRefreshTime1 = millis(); //elmnetjük az időt hogy innen számoljuk
    }
  }

  else {
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
  }
  proba.ontoz_ki();
  ubidots.loop();// subcribe adatok lekérése
  delay (500);
}
