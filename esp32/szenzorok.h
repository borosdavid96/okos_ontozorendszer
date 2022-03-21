
bool beavatkozas[4];

Ubidots ubidots(UBIDOTS_TOKEN);


struct struktura1 {
   int pumpa;
   int homerseklet; //weblapos meghatározott hőmérséklet
   int paratartalom;
   int Tartomany = 0;
   int ontozesidotartam = 5;
   int sleep_time = 30;
};


 struct struktura2 {

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
  

public:

  struktura1 beolvas;
  struktura2 szenzor;
  struktura2 elozo;


  void update() {
     dht_beolvas();
     fold_hom();
     foldnedv_olvas();
     vizszint_olvas();
     vizszint_kiir();
   
  }
  
  void print()
  {
    dht_kiir();
    fold_kiir();
    soilmoisture_print();
    vizszint_kiir();
  }
  void vezerles(){
 //vezérlés 
    if ( szenzor.soilmoisture_value < beolvas.pumpa ) { //x másodpercenként öntöz egyszer x ideig
      beavatkozas[0] = true;
      ontoz();
    }
    else {
      beavatkozas[0] = false;
    }
    if ( szenzor.dht_adat[1] < beolvas.homerseklet) { //addig világít amíg x másodperces mintavételezéssel jó nem lesz
      beavatkozas[1] = true;
      vilagitas();
    }
    else {
      beavatkozas[1] = false;
      digitalWrite(relay2, HIGH);
    }
    if ( szenzor.dht_adat[0] < beolvas.paratartalom) {
      beavatkozas[2] = true;
      digitalWrite(relay3, LOW);
      std::cout << "Parasító megy" << std::endl;
    }
    else {
      beavatkozas[2] = false;
      digitalWrite(relay3, HIGH);
    }
    if (float(beolvas.homerseklet + beolvas.Tartomany) <  proba.szenzor.dht_adat[1]) {
      beavatkozas[3] = true;
      //digitalWrite(relay4,LOW); //akkor kell csak ha a második nincs
      std::cout << "Venti megy" << std::endl;
    }
    else {
      beavatkozas[3] = false;
      //digitalWrite(relay4,HIGH); //akkor kell csak ha a második eszköz nincs
    }
 
    }
    void publikalas(){
        if (pub == 1) { //publikalas be-ki

      // Insert your variable Labels and the value to be sent
      ubidots.add("soil",  szenzor.soilmoisture_value); //(változónév a felhőben,változónév lokálisan)
      ubidots.add("dht11-parat",  szenzor.dht_adat[0]);
      ubidots.add("dht11-hom",  szenzor.dht_adat[1]);
      ubidots.publish(DEVICE1);//az eszköz neve ahová publikálni szeretnénk
      delay(1000); // egy másodperc alatt maximum 4et tudunk publikálni ezért utána időt kell hagyni különben csak néha vagy nem lesz publikálva ami ezen túl csúszik
      ubidots.add("foldhom", szenzor.fold_homerseklet);
      ubidots.add("vizszint",  szenzor.vizertek);
      ubidots.publish(DEVICE1);
      std::cout << "Publikalas vegrehajtva!" << std::endl;
      }
    }
};




// dht11
bool Szenzorok::dht_beolvas()
{

  szenzor.dht_adat[0] = dht.readHumidity();
  szenzor.dht_adat[1] = dht.readTemperature();

  // Check if any reads isnt a number
  if (isnan( szenzor.dht_adat[0]) || isnan( szenzor.dht_adat[1]))
  {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  return 1;
}

void Szenzorok::dht_kiir()
{
  Serial.print("Humidity: ");
  Serial.print( szenzor.dht_adat[0]);
  Serial.println(" %");
  Serial.print("Temperature: ");
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
  Serial.print("Nedvesseg:");
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
  // print out the button's state
  std::cout << "Vizerzekelo:" <<szenzor.vizertek << std::endl;
}
