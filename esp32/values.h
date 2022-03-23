//vizszint
const int BUTTON_PIN = 21; // GIOP21 pin connected to vizszintmero
//soilmoisture-hez
#define SensorPin 34  //elvileg bármelyik kiosztható esp32pin lehet analóg is
const int AirValue = 3600;   
const int WaterValue = 1680;  
int soilmoisturepercent=0;
int soilMoistureValue = 0;
//dht11
#include "DHT.h"
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

 //onewire
#include "OneWire.h"
#include "DallasTemperature.h"
OneWire oneWire(22);
DallasTemperature tempSensor(&oneWire);

//relék
#define relay1 5
#define relay2 18
#define relay3 26
#define relay4 27

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

#define uS_TO_M 60000000  //Conversion micro seconds to seconds
#define DEBUG false

int uresfutas_szamlalo = 0;
