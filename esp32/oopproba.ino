
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

Szenzorok proba;

unsigned long lastRefreshTime = 0;

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
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    proba.beolvas.homerseklet = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany:" << value << std::endl;
    proba.beolvas.Tartomany = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/paratartalom/lv")) {
    std::cout << "A lekérdezett parasítási határérték:" << value << std::endl;
    proba.beolvas.paratartalom = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ontozesidotartam/lv")) {
    std::cout << "A lekérdezett ontozesidotartam:" << value << std::endl;
    proba.beolvas.ontozesidotartam = value;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/sleepingtime/lv")) {
    std::cout << "A lekérdezett sleep_time:" << value << std::endl;
    proba.beolvas.sleep_time = value;
  }

  /*switch(topic) {
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
  */

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


  

  Serial.println("Elso futas, 3ms várakozás"); //szenzorok inicializálására időt kell hagyni induláskor
  delay (3000);

  lastRefreshTime = millis();
  
  
}


void loop()
{

 proba.ujracsatlakozas();

  if (labs(millis() - lastRefreshTime) >= REFRESH_INTERVAL) // abszolútértéke a kettő különbségének amíg kisebb mint a kijelölt időtartomány nem fog futni , triggers the routine every x seconds
  {


    
    proba.update();
    proba.print();
    proba.vezerles();
    proba.publikalas();
    proba.inaktiv();

    /*
    //soil-be
    proba.szenzor.soilmoisture_value = foldnedv_olvas();

    //dht11-be
    //dht_beolvas( proba.szenzor.dht_adat);

    //onewire-be
     proba.szenzor.fold_homerseklet = fold_hom();//fold hom olvasása

    //vizszint-be
     proba.szenzor.vizertek = vizszint_olvas();

    //soil-kiir
     proba.szenzor.soilmoisture_value = szazalekosit(proba.szenzor.soilmoisture_value);
    soilmoisture_print( proba.szenzor.soilmoisture_value);

    //dht11-kiir
    dht_kiir( proba.szenzor.dht_adat);

    //onewire-kiir
    fold_kiir( proba.szenzor.fold_homerseklet);

    //vizszint-kiir
    vizszint_kiir( proba.szenzor.vizertek);
    */
/*
    //vezérlés majd ide
    if ( proba.szenzor.soilmoisture_value < proba.beolvas.pumpa ) { //x másodpercenként öntöz egyszer x ideig
      beavatkozas[0] = true;
      ontoz();
    }
    else {
      beavatkozas[0] = false;
    }
    if ( proba.szenzor.dht_adat[1] < proba.beolvas.homerseklet) { //addig világít amíg x másodperces mintavételezéssel jó nem lesz
      beavatkozas[1] = true;
      vilagitas();
    }
    else {
      beavatkozas[1] = false;
      digitalWrite(relay2, HIGH);
    }
    if ( proba.szenzor.dht_adat[0] < proba.beolvas.paratartalom) {
      beavatkozas[2] = true;
      digitalWrite(relay3, LOW);
      std::cout << "Parasító megy" << std::endl;
    }
    else {
      beavatkozas[2] = false;
      digitalWrite(relay3, HIGH);
    }
    if (float(proba.beolvas.homerseklet + proba.beolvas.Tartomany) <  proba.szenzor.dht_adat[1]) {
      beavatkozas[3] = true;
      //digitalWrite(relay4,LOW); //akkor kell csak ha a második nincs
      std::cout << "Venti megy" << std::endl;
    }
    else {
      beavatkozas[3] = false;
      //digitalWrite(relay4,HIGH); //akkor kell csak ha a második eszköz nincs
    }
*/
/*
     if (pub == 1) { //publikalas be-ki

      // Insert your variable Labels and the value to be sent
      ubidots.add("soil",  proba.szenzor.soilmoisture_value); //(változónév a felhőben,változónév lokálisan)
      ubidots.add("dht11-parat",  proba.szenzor.dht_adat[0]);
      ubidots.add("dht11-hom",  proba.szenzor.dht_adat[1]);
      ubidots.publish(DEVICE1);//az eszköz neve ahová publikálni szeretnénk
      delay(1000); // egy másodperc alatt maximum 4et tudunk publikálni ezért utána időt kell hagyni különben csak néha vagy nem lesz publikálva ami ezen túl csúszik
      ubidots.add("foldhom",  proba.szenzor.fold_homerseklet);
      ubidots.add("vizszint",  proba.szenzor.vizertek);
      ubidots.publish(DEVICE1);
      std::cout << "Publikalas vegrehajtva!" << std::endl;

    }
    */
   

    lastRefreshTime = millis(); //elmnetjük az időt hogy innen számoljuk
  }

    ubidots.loop();// subcribe adatok lekérése
  

  delay (1000);
}
