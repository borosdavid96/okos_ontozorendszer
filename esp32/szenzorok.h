
//dht11 
int dht_beolvas(float * h){
   
    h[0] = dht.readHumidity();
    h[1] = dht.readTemperature();
  
     // Check if any reads isnt a number
    if (isnan(h[0]) || isnan(h[1]) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
     }
return 1;
 }

 void dht_kiir(float * h){
  Serial.print("Humidity: ");
  Serial.print(h[0]);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(h[1]);
  Serial.println("°C ");
}

//foldhom
 float fold_hom (){
 tempSensor.requestTemperaturesByIndex(0);
 float fold = tempSensor.getTempCByIndex(0);
 return fold;
 }
 void fold_kiir (float data){

std::cout<<"Föld homerseklet: "<< data << " C" << std::endl;

}

//soilmoisture
int foldnedv_olvas(){
  
 int sensorValue = analogRead(SensorPin);
 return  sensorValue; 
}

int szazalekosit(int adat){
int soilmoisturepercent = map(adat, AirValue, WaterValue, 0, 100); //map tartományt alakít át, 1. a mért érték, 2. tól,3. ig,4. a tól megfeleleője,5 az ig-megfeleleője 
if(soilmoisturepercent > 100)//ha több mint 100 akkor 100%
  {
   soilmoisturepercent=100;
}
 else if(soilmoisturepercent <0)//ha kevesebb mint 0 akkor 0%
  {
   soilmoisturepercent=0;
}
  return  soilmoisturepercent;//függvény a %os átalakításra
  
}
void soilmoisture_print(int data){
  Serial.print("Nedvesseg:");
  Serial.print(data);
  Serial.println("%");
  }

//vizszint
int vizszint_olvas (){
 int buttonState = digitalRead(BUTTON_PIN);
 return buttonState;
}
void vizszint_kiir(int ertek){
  // print out the button's state 
  std::cout<<"Vizerzekelo:"<<ertek<<std::endl;
}
