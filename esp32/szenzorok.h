
struct struktura1 { //weblapról
   int pumpa;
   int homerseklet; //weblapos meghatározott hőmérséklet
   int paratartalom;
   int Tartomany = 0;
   int ontozesidotartam = 5;
   int sleep_time = 30;
};

 struct struktura2 { //szenzorból

  int soilmoisture_value;
  float fold_homerseklet;
  float dht_adat[2];
  int vizertek ;
};


class Szenzorok
{
  bool dht_beolvas();
  void vizszint_olvas();
  void fold_hom();
  void foldnedv_olvas();
  void dht_kiir();
  void vizszint_kiir();
  void soilmoisture_print();
  void fold_kiir();
  int szazalekosit(int adat);
  bool pub = 1; //publikáció kikapcsolása 0 , 1 publikálás
  int maximum_uresfutas=10;
  void sleep ();
  void ontoz();
  
  unsigned long lastRefreshTime2 = 0;
  int alvas = 0;
 
  bool beavatkozas[4];
  
public:

  struktura1 beolvas;
  struktura2 szenzor;

  void ujracsatlakozas();
  void relays_off();
  void subscribe_all();
  void vezerles();
  void publikalas();
  void inaktiv();
  void ontoz_ki();
  int uresfutas_szamlalo = 0;
  bool lekerdezesek[6];


  
  void update() {
     dht_beolvas();
     fold_hom();
     foldnedv_olvas();
     vizszint_olvas();
  }
  void print()
  {
    dht_kiir();
    fold_kiir();
    soilmoisture_print();
    vizszint_kiir();
  }
        
};

// dht11
bool Szenzorok::dht_beolvas()
{
  szenzor.dht_adat[0] = dht.readHumidity();
  szenzor.dht_adat[1] = dht.readTemperature();
  // ha az érték nem szám
  if (isnan( szenzor.dht_adat[0]) || isnan( szenzor.dht_adat[1]))
  {
    Serial.println("Az adatbeolvasás a DHT szenzorból sikertelen volt!");
    return 0;
  }
  return 1;
}
void Szenzorok::dht_kiir()
{
  Serial.print("Levegő páratartalma: ");
  Serial.print( szenzor.dht_adat[0]);
  Serial.println(" %");
  Serial.print("Levegő hőmérséklete: ");
  Serial.print( szenzor.dht_adat[1]);
  Serial.println("°C ");
}
// foldhom beolvasas , kiiras
void Szenzorok::fold_hom()
{
  tempSensor.requestTemperaturesByIndex(0);
  szenzor.fold_homerseklet = tempSensor.getTempCByIndex(0); 
}
void Szenzorok::fold_kiir()
{
  std::cout << "Föld homerseklet: " << szenzor.fold_homerseklet << " °C" << std::endl;
}
// soilmoisture beolvasas , kiiras
void Szenzorok::foldnedv_olvas()
{
   szenzor.soilmoisture_value = szazalekosit( analogRead(SensorPin));
}

int Szenzorok::szazalekosit(int adat)
{                                                                    
  int soilmoisturepercent = map(adat, AirValue, WaterValue, 0, 100); // a map tartományt alakít át, 1. a mért érték, 2. tól,3. ig,4. a tól megfelelője,5 az ig-megfeleleője
  if (soilmoisturepercent > 100)                                     // ha több mint 100 akkor 100%
  {
    soilmoisturepercent = 100;
  }
  else if (soilmoisturepercent < 0) // ha kevesebb mint 0 akkor 0%
  {
    soilmoisturepercent = 0;
  }
  return soilmoisturepercent; // függvény a %os átalakításra
}
void Szenzorok::soilmoisture_print()
{
  Serial.print("Földnedvesség:");
  Serial.print( szenzor.soilmoisture_value);
  Serial.println("%");
}

// vizszint
void Szenzorok::vizszint_olvas()
{
  szenzor.vizertek = digitalRead(BUTTON_PIN);
}
void Szenzorok::vizszint_kiir()
{
  std::cout << "Vizérzékelő állása:" <<szenzor.vizertek << std::endl;
}


void Szenzorok::ujracsatlakozas(){
   if (!ubidots.connected()) //ujracsatlakozas
  {
    relays_off();
    ubidots.reconnect();
    subscribe_all();
  }
  }
void Szenzorok::relays_off(){
  digitalWrite(relay1, HIGH); //fordított logika miatt magasra állítás (tehát kikapcsolt állapotban tartás
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void Szenzorok::subscribe_all(){
  ubidots.subscribeLastValue(DEVICE1, "pumpa");
  ubidots.subscribeLastValue(DEVICE1, "homerseklet");
  ubidots.subscribeLastValue(DEVICE2, "tartomany");
  ubidots.subscribeLastValue(DEVICE1, "paratartalom"); 
  ubidots.subscribeLastValue(DEVICE2, "ontozesidotartam");
  ubidots.subscribeLastValue(DEVICE2, "sleepingtime");
}
void Szenzorok::ontoz() {
  digitalWrite(relay1, LOW);
  Serial.println("Ontozes megy.");
 lastRefreshTime2=millis();

}
void Szenzorok::ontoz_ki() {
  vizszint_olvas();
  if(szenzor.vizertek && beavatkozas[0]==true){
        std::cout << "Pumpa leáll mivel a vízszint túl alacsony!" << std::endl;
        digitalWrite(relay1, HIGH);
        beavatkozas[0] = false;
      }
  else if(beavatkozas[0]==true && (labs(millis() - lastRefreshTime2) >= beolvas.ontozesidotartam * 1000) ){
  digitalWrite(relay1, HIGH);
  std::cout << "Öntözés leáll." << std::endl;
   beavatkozas[0] = false;
}
}
void Szenzorok::vezerles(){
   //vezérlés 
    if ((szenzor.soilmoisture_value < beolvas.pumpa) && !szenzor.vizertek) { //x másodpercenként öntöz egyszer x ideig
      beavatkozas[0] = true;
      ontoz();
    }
    else if (szenzor.vizertek){
      std::cout << "Öntözés szükséges viszont nem lehetséges mivel nincs elég víz." << std::endl;
      }
 
    if ( szenzor.dht_adat[1] < beolvas.homerseklet) { //addig világít amíg x másodperces mintavételezéssel jó nem lesz
      beavatkozas[1] = true;
        digitalWrite(relay2, LOW);
    std::cout << "Fűtés megy." << std::endl;
    }
    else if(beavatkozas[1] == true) {
      beavatkozas[1] = false;
      digitalWrite(relay2, HIGH);
      std::cout << "Fűtés leáll." << std::endl;
    }
    if ( szenzor.dht_adat[0] < beolvas.paratartalom) {
      beavatkozas[2] = true;
      digitalWrite(relay3, LOW);
      std::cout << "Parasítás megy." << std::endl;
    }
    else if(beavatkozas[2] == true) {
      beavatkozas[2] = false;
      digitalWrite(relay3, HIGH);
      std::cout << "Parasítás leáll." << std::endl;
    }
    if (float(beolvas.homerseklet + beolvas.Tartomany) <=  szenzor.dht_adat[1]) {
      beavatkozas[3] = true;
      //digitalWrite(relay4,LOW); //akkor kell csak ha a második nincs
      std::cout << "Hűtés megy." << std::endl;
    }
    else if(beavatkozas[3] == true) {
      beavatkozas[3] = false;
      std::cout << "Hűtés leáll." << std::endl;
      //digitalWrite(relay4,HIGH); //akkor kell csak ha a második eszköz nincs
    }
  }
  void Szenzorok::publikalas(){
          if (pub == 1) { //publikalas be-ki
      // Insert your variable Labels and the value to be sent
      ubidots.add("soil",  szenzor.soilmoisture_value); //(változónév a felhőben,változónév lokálisan)
      ubidots.add("dht11-parat",  szenzor.dht_adat[0]);
      ubidots.add("dht11-hom",  szenzor.dht_adat[1]);
      ubidots.publish(DEVICE1);//az eszköz neve ahová publikálni szeretnénk
      delay(1000); // egy másodperc alatt maximum 4et tudunk publikálni ezért utána időt kell hagyni különben csak ritkán vagy egyáltalán nem lesz publikálva ami ezen túl csúszik
      ubidots.add("foldhom", szenzor.fold_homerseklet);
      ubidots.add("vizszint",  szenzor.vizertek);
      ubidots.publish(DEVICE1);
      std::cout << "Publikalas vegrehajtva!" << std::endl;
      }
    }
    void Szenzorok::inaktiv (){
       
    if (beavatkozas[0] + beavatkozas[1] + beavatkozas[2] + beavatkozas[3] < 1) { // ha nem kellett beavatkozás akkor számláló++ ha kellett újraindul a számolás
      uresfutas_szamlalo += 1;
      std::cout << "Üres futás " << uresfutas_szamlalo << " alkalommal" << std::endl;

      if (uresfutas_szamlalo > maximum_uresfutas) { //alvas ha x szer nincs semmi beavatkozás
          sleep();
        }
    }
    else {
      uresfutas_szamlalo = 0;
    }
      }

      void Szenzorok::sleep () {
  //Set sleep timer to x seconds
  alvas = 1;
  ubidots.add("adat", alvas); //1 küldése hogy a második mikrokontroller aludjon
  ubidots.publish(DEVICE1);
  delay(5000);//felhős oldal miatt kell mert nem érzékeli új adatnak mindig ha ennél gyorsabban küldöm
  alvas = 0;
  ubidots.add("adat", alvas); //0 küldése utána ha esetleg előbb ébredne fel a második ne aludjon az újbóli lekérdezése miatt
  ubidots.publish(DEVICE1);
  esp_sleep_enable_timer_wakeup(beolvas.sleep_time * uS_TO_M);
  Serial.println(" ESP32 going to sleep for " + String(beolvas.sleep_time) +
                 " Minutes");
  esp_deep_sleep_start();  //Go to sleep now
}
