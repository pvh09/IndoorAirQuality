#include "SoftwareSerial.h"
#include "MHZ19.h"   // https://github.com/WifWaf/MH-Z19
#include "PMS.h"     //https://github.com/fu-hsi/pms
#include "dht.h"     // https://github.com/RobTillaart/DHTlib
#include "DS3231.h"  // http://www.rinkydinkelectronics.com/library.php?id=73
#include <MQUnifiedsensor.h>
#include <RTClib.h>

//Definitions
#define placa "Arduino Mini Pro"
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino 
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10    // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6  //RS / R0 = 3.6 ppm  
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type); 

DateTime now;
RTC_DS3231 rtc;

#define led 13
#define tvocPin 7  // VOC sensor activation
#define dht22 5 // DHT22 temperature and humidity sensor

dht DHT; // Creats a DHT object

MHZ19 myMHZ19;    // CO2 Sensor
SoftwareSerial co2Serial(2, 3);  // (RX, TX) MH-Z19 serial

SoftwareSerial pmsSerial(8, 9); // Particulate Matter sensor
PMS pms(pmsSerial);
PMS::DATA data;

int readDHT, temp, hum;
int CO2;
int CO;
int tvoc;
int pm25;
float tempError = 4;
int hours, minutes, second;
int previousMinutes = 1;
String timeString;
String receivedData = "Z";
char t[32];

void setup() {
  Serial.begin(9600);
  // Device to serial monitor feedback
  pinMode(6, OUTPUT);
  pinMode(tvocPin, OUTPUT);

  // Warming up sensors
  //digitalWrite(6, HIGH);        // Ozone sensor
  digitalWrite(tvocPin, HIGH);  // TVOC sensor
  delay(3 * 1000); // delay 7 seconds
  digitalWrite(6, LOW);
  digitalWrite(tvocPin, LOW);

  // Initialize all sensors
  rtc.begin();
  co2Serial.begin(9600);
  pmsSerial.begin(9600);
  myMHZ19.begin(co2Serial);
  myMHZ19.setFilter(true, true);
  myMHZ19.autoCalibration(true);

  //MQ 135 Setup Section 
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(605.18); 
  MQ135.setB(-3.937);
  MQ135.init(); 
  Serial.println(F("Calibrating please wait."));
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
  }
  MQ135.setR0(calcR0/10);
  if(isinf(calcR0)) {Serial.println(F("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply")); while(1);}
  if(calcR0 == 0){Serial.println(F("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 

  // setup RTC module
  if (! rtc.begin()) 
  {
    Serial.println(F(" RTC Module not Present"));
    while (1);
  }
  if (rtc.lostPower()) 
  {
    Serial.println(F("RTC power failure, reset the time!"));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  //Read temperature and humidity from DHT22 sensor
  readDHT = DHT.read22(dht22); 
  temp = (DHT.temperature - tempError); // Gets the values of the temperature
  hum = DHT.humidity; // Gets the values of the humidity

 tvoc = analogRead(A1); // Please note that we are only reading raw data from this sensor, not ppm or ppb values. Just analog values from 0 to 1024. Higher values means there is a presence of VOC 
  digitalWrite(tvocPin, LOW);

  //Read MHZ19 - CO2 sensor for 3 seconds - if we don't use a blocking method with the while loop we won't get values from the sensor.
  co2Serial.listen();
  CO2 = myMHZ19.getCO2(true, true); // Request CO2 (as ppm)

  //Read Particulate Matter sensor for 2 seconds
  pmsSerial.listen();
  pms.readUntil(data);
  pm25 = data.PM_AE_UG_2_5;
  now = rtc.now();
  
  //MQ Reading and updating section
  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
  CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  // Send the data to the Nextion display
  Serial.print(temp);
  Serial.print(F("|"));
    
  Serial.print(hum);
  Serial.print(F("|"));
    
  Serial.print(CO2);
  Serial.print(F("|"));
  
  Serial.print(pm25);
  Serial.print(F("|"));

  Serial.print(tvoc);
  Serial.print(F("|"));
    
  Serial.print(CO);
  Serial.print(F("|"));

  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());   
  Serial.println(t);
  delay(4000);
}
