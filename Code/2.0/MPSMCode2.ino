/*
  Projekt: BuoyBoys24
  Names: Wilson Dhalwani & Vladyslav Kharechko & Peter Rappazzo 
  Advisor: Mr. Liva
  Class: Engineering Research
  Date: 02/10/2023 
  Last Updated: 02/11/2023
*/

//Libraries
#include <DHT.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include "Ubidots.h"
#include <Ubidots.h>

//Constants, announcements, and the other fun stuff that belongs here; we live in a society
//DHT22
#define DHTPIN 14          //digiPin
#define DHTTYPE DHT22      // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);  //Initialize DHT sensor for normal 16mhz Arduino
float hum;                 //Stores humidity value
float temp;                //Stores temperature value
int tempF;                 //tempvalue in Farenheit
//TSL2591
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
int lux;  //lux
//battLife
int prcnt;  //batterylife percent
int raw;    //raw read value from A0 or 17
int volt;   //inital volt conversion with decent degree of errorkvk
int volt2;  //secondary volt conversion with minimal degree of error
//wifi
const char* UBIDOTS_TOKEN = "BBFF-kdcXABSmCfmapAA4LTDLU59WMhIBY7";  //UbidotsToken
const char* WIFI_SSID = "dhalwani";                                 //WifiSSID
const char* WIFI_PASS = "monish30";                                 //WifiPass
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

//Config Functions
void displaySensorDetails(void) {
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print(F("Sensor:       "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:   "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:    "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:    "));
  Serial.print(sensor.max_value);
  Serial.println(F(" lux"));
  Serial.print(F("Min Value:    "));
  Serial.print(sensor.min_value);
  Serial.println(F(" lux"));
  Serial.print(F("Resolution:   "));
  Serial.print(sensor.resolution, 4);
  Serial.println(F(" lux"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}
void configureSensor(void) {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);  // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)Serial.println(F("------------------------------------"));
  Serial.print(F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch (gain) {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print(F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}
void TSLconfig(void) {
  Serial.println(F("Starting Adafruit TSL2591 Test!"));

  if (tsl.begin()) {
    Serial.println(F("Found a TSL2591 sensor"));
  } else {

    Serial.println(F("No sensor found ... check your wiring?"));
    while (1)
      ;
  }
  /* Display some basic information on this sensor */
  displaySensorDetails();
  /* Configure the sensor */
  configureSensor();
}
void initializerConfig(void) {
  //initilize sensors
  dht.begin();
  TSLconfig();
  //wifi initialize
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  //ubidots.setDebug(true);                        //Uncomment for printing debug msgs
}

//Processing Functions
//TSL2591 Calculation Functions
//Shows how to perform a basic read on visible, full spectrum or infrared light (returns raw 16-bit ADC values)
void simpleRead(void) {
  // Simple data read example. Just read the infrared, fullspecrtrum diode
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  //uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  //uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

  Serial.print(F("[ "));
  Serial.print(millis());
  Serial.print(F(" ms ] "));
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);
}
//Show how to read IR and Full Spectrum at once and convert to lux
void advancedRead(void) {
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  // Serial.print(F("[ "));
  // Serial.print(millis());
  // Serial.print(F(" ms ] "));
  // Serial.print(F("IR: "));
  // Serial.print(ir);
  // Serial.print(F("  "));
  // Serial.print(F("Full: "));
  // Serial.print(full);
  // Serial.print(F("  "));
  // Serial.print(F("Visible: "));
  // Serial.print(full - ir);
  // Serial.print(F("  "));
  // Serial.print(F("Lux: "));
  // Serial.println(tsl.calculateLux(full, ir), 6);
  lux = tsl.calculateLux(full, ir);
}
//Performs a read using the Adafruit Unified Sensor API.
void unifiedSensorAPIRead(void) {
  /* Get a new sensor event */
  sensors_event_t event;
  tsl.getEvent(&event);
  /* Display the results (light is measured in lux) */
  Serial.print(F("[ "));
  Serial.print(event.timestamp);
  Serial.print(F(" ms ] "));
  if ((event.light == 0) | (event.light > 4294966000.0) | (event.light < -4294966000.0)) {

    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.println(F("Invalid data (adjust gain or timing)"));
  } else {
    Serial.print(event.light);
    Serial.println(F(" lux"));
  }
}
void lightRead(void) {
  //simpleRead();
  advancedRead();
  //unifiedSensorAPIRead();
  Serial.print("Lux: ");
  Serial.println(lux);
}
//DHT22 Calculation Functions
void humidTempRead(void) {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  int tempF = (temp * 9 / 5) + 32;
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %");
  //Serial.print(" %, Temp: ");
  //Serial.print(temp);
  //Serial.print(" Celsius");
  Serial.print(", Temperature: ");
  Serial.print(tempF);
  Serial.println(" F");
}
//BatteryLife Calculation Functions
void battRead(void) {
  //add calculations for battery life
  int raw = analogRead(A0);
  //Serial.println(raw);
  float volt = 0.0042 * raw - 0.0022;  //found using excell graphing; trendline
  //Serial.println(volt);
  float volt2 = 1.0007 * volt - 0.0802;  //improves calculation accuracy; used excel graphing; trendline
  //Serial.println(volt2);
  int prcnt = map(raw, 0, 1024, 0, 100);
  int battery = prcnt;
  if (volt2 < 3.0 || volt2 > 4.2) {  //debug check for volatge check
    Serial.println("DANGER! INSUFFICENT VOLTAGE!");
  } else {
    Serial.print("Battery Life: ");
    Serial.print(prcnt);
    Serial.println("%");
  }
}
//Wifi&Server Send Functions
void packetSend(void) {

  //battery nonsense
  int raw = analogRead(A0);
  //Serial.println(raw);
  float volt = 0.0042 * raw - 0.0022;  //found using excell graphing; trendline
  //Serial.println(volt);
  float volt2 = 1.0007 * volt - 0.0802;  //improves calculation accuracy; used excel graphing; trendline
  //Serial.println(volt2);
  int prcnt = map(raw, 0, 1024, 0, 100);
  int battery = prcnt;

  ubidots.add("Temperature", (temp * 9 / 5) + 32);  //converts from C to F (for some reason it doesn't work if I convert it above)
  ubidots.add("Humidity", hum);
  ubidots.add("BatteryLife", battery);
  ubidots.add("Lux", lux);

  bool bufferSent = false;
  bufferSent = ubidots.send();  // Will send data to a device label that matches the device Id

  if (bufferSent) {
    // Do something if values were sent properly
    Serial.println("Packets Sent!");
  }
}

//Board Startup Function
void setup() {
  //initialize board
  Serial.begin(115200);  //baud rate
  //initliize eveyrthing else
  initializerConfig();
}

//Board Activity Function
void loop() {
  //Read and Print Temp/Humidiity Info
  humidTempRead();

  //Read and Print Light Info
  lightRead();

  //Read and Print BatteryLife Info
  battRead();

  //UbiDots Send Schpiel
  packetSend();

  //generalDelay
  delay(60000);  //dely 2.5 sec
}