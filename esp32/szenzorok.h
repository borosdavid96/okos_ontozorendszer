
struct struktura_be { //weblapról jövő adatok
  int pumpa;
  int homerseklet;
  int paratartalom;
  int Tartomany;
  int ontozesidotartam;
  int sleep_time;
  bool ket_eszkoz_e;
  bool szelloztetes;
};

struct struktura_ki { //szenzorból kapott adatok

  int soilmoisture_value;
  float fold_homerseklet;
  float dht_adat[2];
  int vizertek ;
};


class Szenzorok
{
    struktura_be beolvas;
    struktura_ki szenzor;
    
    void sleep ();
    int adatszam = 7;
    int maximum_uresfutas = 10;
    int alvas = 0;
    bool beavatkozas[4];
    bool lekerdezesek[9];
    int lastRefreshTime2=0;

  public:
    Szenzorok() {
      for (int i; i < sizeof(lekerdezesek); i++){
        lekerdezesek[i] = false;
      }
    }
    
    bool pub = 1; //publikáció kikapcsolása 0 , 1 publikálás
    void relek_ki_kiveve();
    void adat_lekeres_ellenorzes();
    void ontoz();
    void uzenetek_beolvasasa(char* topic, int value);
    void ujracsatlakozas();
    void relek_ki();
    void subscribe_all();
    void vezerles();
    void publikalas();
    void inaktiv();
    void ontoz_ki();
    int uresfutas_szamlalo = 0;

    bool dht_beolvas();
    void vizszint_olvas();
    void fold_hom();
    void foldnedv_olvas();
    void dht_kiir();
    void vizszint_kiir();
    void soilmoisture_print();
    void fold_kiir();
    int szazalekosit(int adat);

    int get_ket_eszkoz_e();
    int get_szelloztetes();
    void uresfutas_nullazasa();

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
    std::cout << "Az adatbeolvasás a DHT szenzorból sikertelen volt!"<< std::endl;
    return 0;
  }
  return 1;
}
void Szenzorok::dht_kiir()
{
  std::cout << "Levegő páratartalma: "<< szenzor.dht_adat[0]<<" %"<< std::endl;
  std::cout << "Levegő hőmérséklete: "<< szenzor.dht_adat[1]<<" °C"<< std::endl;
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
  std::cout << "Földnedvesség:" << szenzor.soilmoisture_value << "%" << std::endl;
}

// vizszint
void Szenzorok::vizszint_olvas()
{
  szenzor.vizertek = digitalRead(BUTTON_PIN);
}
void Szenzorok::vizszint_kiir()
{
  std::cout << "Vizérzékelő állása:" << szenzor.vizertek << std::endl;
}


void Szenzorok::ujracsatlakozas() {
  if (!ubidots.connected()) //ujracsatlakozas
  {
    relek_ki();
    ubidots.reconnect();
    subscribe_all();
  }
}
void Szenzorok::relek_ki() {
  digitalWrite(relay1, HIGH); //fordított logika miatt magasra állítás (tehát kikapcsolt állapotban tartás
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}
void Szenzorok::relek_ki_kiveve() {
  digitalWrite(relay2, HIGH);//fordított logika miatt magasra állítás (tehát kikapcsolt állapotban tartás
  digitalWrite(relay3, HIGH);
}
void Szenzorok::subscribe_all() {
  ubidots.subscribeLastValue(DEVICE1, "pumpa");
  ubidots.subscribeLastValue(DEVICE1, "homerseklet");
  ubidots.subscribeLastValue(DEVICE2, "tartomany");
  ubidots.subscribeLastValue(DEVICE1, "paratartalom");
  ubidots.subscribeLastValue(DEVICE2, "ontozesidotartam");
  ubidots.subscribeLastValue(DEVICE2, "alvasi_ido");
  ubidots.subscribeLastValue(DEVICE2, "ket_eszkoz_e");
  ubidots.subscribeLastValue(DEVICE1, "szelloztetes");

}
void Szenzorok::ontoz() {
  if ((szenzor.soilmoisture_value < beolvas.pumpa) && !szenzor.vizertek) { //x másodpercenként öntöz egyszer x ideig
    beavatkozas[0] = true;
    digitalWrite(relay1, LOW);
    std::cout << "Öntözés megy." << std::endl;
    lastRefreshTime2 = millis();
  }
  else if (szenzor.vizertek) {
    std::cout << "Öntözés szükséges viszont nem lehetséges mivel nincs elég víz." << std::endl;
  }


}
void Szenzorok::ontoz_ki() {
  vizszint_olvas();
  if (szenzor.vizertek && beavatkozas[0] == true) {
    std::cout << "Pumpa leáll mivel a vízszint túl alacsony!" << std::endl;
    digitalWrite(relay1, HIGH);
    beavatkozas[0] = false;
  }
  else if (beavatkozas[0] == true && (labs(millis() - lastRefreshTime2) >= beolvas.ontozesidotartam * 1000) ) {
    digitalWrite(relay1, HIGH);
    std::cout << "Öntözés leáll." << std::endl;
    beavatkozas[0] = false;
  }
}
void Szenzorok::vezerles() {
  //vezérlés
  ontoz();

  if ( szenzor.dht_adat[1] < beolvas.homerseklet) { //addig világít amíg x másodperces mintavételezéssel jó nem lesz
    beavatkozas[1] = true;
    digitalWrite(relay2, LOW);
    std::cout << "Fűtés megy." << std::endl;
  }
  else if (beavatkozas[1] == true) {
    beavatkozas[1] = false;
    digitalWrite(relay2, HIGH);
    std::cout << "Fűtés leáll." << std::endl;
  }
  if ( szenzor.dht_adat[0] < beolvas.paratartalom) {
    beavatkozas[2] = true;
    digitalWrite(relay3, LOW);
    std::cout << "Parasítás megy." << std::endl;
  }
  else if (beavatkozas[2] == true) {
    beavatkozas[2] = false;
    digitalWrite(relay3, HIGH);
    std::cout << "Parasítás leáll." << std::endl;
  }
  if (float(beolvas.homerseklet + beolvas.Tartomany) <=  szenzor.dht_adat[1]) {
    beavatkozas[3] = true;
    std::cout << "Hűtés megy." << std::endl;
    if (!beolvas.ket_eszkoz_e) {
      digitalWrite(relay4, LOW);
    } //akkor kell csak ha a második nincs
  }

  else if (beavatkozas[3] == true) {
    beavatkozas[3] = false;
    std::cout << "Hűtés leáll." << std::endl;
      digitalWrite(relay4, HIGH); //működés közben történő nyomkodás miatt mindig ki kell kapcsolni    
  }
}
void Szenzorok::publikalas() {
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
void Szenzorok::inaktiv () {

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

void Szenzorok::sleep () {//alvás
  if (beolvas.ket_eszkoz_e) {
    alvas = 1;
    ubidots.add("adat", alvas); //1 küldése hogy a második mikrokontroller aludjon
    ubidots.publish(DEVICE1);
    delay(5000);//felhős oldal miatt kell mert nem érzékeli új adatnak mindig ha ennél gyorsabban küldöm
    alvas = 0;
    ubidots.add("adat", alvas); //0 küldése utána ha esetleg előbb ébredne fel a második ne aludjon az újbóli lekérdezése miatt
    ubidots.publish(DEVICE1);
  }
  esp_sleep_enable_timer_wakeup(beolvas.sleep_time * uS_TO_M);
  Serial.println(" Az eszköz aludni fog " + String(beolvas.sleep_time) +
                 " percig.");
  esp_deep_sleep_start();  //Go to sleep now
}
void Szenzorok::uzenetek_beolvasasa(char* topic, int value) {
  if (!strcmp(topic, "/v2.0/devices/esp1/pumpa/lv")) {
    std::cout << "A lekérdezett pumpa értéke:" << value << std::endl;
    beolvas.pumpa = value;
    lekerdezesek[0] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/homerseklet/lv")) {
    std::cout << "A lekérdezett alsó hőmérsékleti hatar:" << value << std::endl;
    beolvas.homerseklet = value;
    lekerdezesek[1] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/tartomany/lv")) {
    std::cout << "A lekérdezett Tartomany:" << value << std::endl;
    beolvas.Tartomany = value;
    lekerdezesek[2] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/paratartalom/lv")) {
    std::cout << "A lekérdezett parasítási határérték:" << value << std::endl;
    beolvas.paratartalom = value;
    lekerdezesek[3] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ontozesidotartam/lv")) {
    std::cout << "A lekérdezett ontozesidotartam:" << value << std::endl;
    beolvas.ontozesidotartam = value;
    lekerdezesek[4] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/alvasi_ido/lv")) {
    std::cout << "A lekérdezett alvasi_ido:" << value << std::endl;
    beolvas.sleep_time = value;
    lekerdezesek[5] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/b4e62d04cda2/ket_eszkoz_e/lv")) {
    std::cout << "A lekérdezett ket_eszkoz_e:" << value << std::endl;
    beolvas.ket_eszkoz_e = value;
    lekerdezesek[6] = true;
  }
  else if (!strcmp(topic, "/v2.0/devices/esp1/szelloztetes/lv")) {
    std::cout << "A lekérdezett szelloztetes:" << value << std::endl;
    beolvas.szelloztetes = value;
    lekerdezesek[7] = true;
  }
}

void Szenzorok::adat_lekeres_ellenorzes() {
  std::cout << "Adatok lekérése, ha nincs meg minden adat nem indul el a program!" << std::endl;
  int i = 0;
  for (i = 0; i < adatszam; i++) {
    while (!lekerdezesek[i]) {
      ubidots.loop();// subcribe adatok lekérése
      delay(100);
    }
  }
}
int Szenzorok::get_ket_eszkoz_e() {
  return beolvas.ket_eszkoz_e;
}
int Szenzorok::get_szelloztetes() {
  return beolvas.szelloztetes;
}
void Szenzorok::uresfutas_nullazasa() {
  if (uresfutas_szamlalo != 0)
    uresfutas_szamlalo = 0;
}
