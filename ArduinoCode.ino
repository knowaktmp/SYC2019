#include <Wire.h>
#include "i2c.h"
#include "i2c_bmp280.h"
#include  <DS3231.h>
#include <MemoryFree.h>
BMP280 bmp280;
DS3231 clock;
RTCDateTime dt;



const int photoResistorPin = A0;
const int voltmeterPin = A1;
const int proximitySensorInputPin = 5;
const int proximitySensorVCC = 6;
const int proximitySensorGND = 4;
const int rtcVCC = 9;
const int rtcGND = 10;
int missionTime = 1000;

int sampleCount = 1;
int samplingTime = 10000; //ms

int initSecond;
int initMinute;
int initHour;

int toEndSecond;
int toEndMinute;
int toEndHour;


bool missionStart = true;

String timeTillEnd;

String data = "";
int counter = 0;

void setup() {

  Serial.begin(9600);

  pinMode(photoResistorPin, INPUT);
  pinMode (voltmeterPin, INPUT);

  pinMode(rtcVCC, OUTPUT);
  pinMode(rtcGND, OUTPUT);
  digitalWrite(rtcVCC, HIGH);
  digitalWrite(rtcGND, LOW);


  pinMode(proximitySensorInputPin, INPUT);
  pinMode(proximitySensorVCC, OUTPUT);
  pinMode(proximitySensorGND, OUTPUT);

  digitalWrite(proximitySensorVCC, HIGH);
  digitalWrite(proximitySensorGND, LOW);

  bmp280.initialize();
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();
  clock.setDateTime(__DATE__, __TIME__);


}

int beginM = 0;
String command = "";
void loop() {


  if(beginM != 1){
    if(Serial.available() > 0){
      String string = "";

      string = Serial.readString();

    // Serial.print(string);
      beginM = 1;
  }}
  else{

 if(Serial.available() > 0){
      command = Serial.readString();
      if(command.substring(0,1) == "1")
        command = command.substring(1);
      if(command == "RETURN"){
     beginM = 1;
     missionStart = true;
     return;
      }
            else if(command.substring(0, 4) == "FREQ"){
          samplingTime = command.substring(5).toInt();
        }
         else if(command.substring(0, 4) == "TIME"){
         missionTime = command.substring(5).toInt();
         missionStart = true;
        }
        
    }

 
  dt = clock.getDateTime();

   

  if (missionStart) {

    initSecond = dt.second;
    initMinute = dt.minute;
    initHour = dt.hour;

    
    int tmpToEndSecond = missionTime % 60;
    int tmpToEndHour = missionTime / 60;
    int tmpToEndMinute = tmpToEndHour % 60;

    tmpToEndHour = tmpToEndHour / 60;

    
    timeTillEnd = "Misja zakonczy siÃ„â„¢ za: " + String(tmpToEndHour) + " h " + String(tmpToEndMinute) + " min " + String(tmpToEndSecond) + " s.";
   // Serial.println(timeTillEnd);                                                                                                                TODO

    if (initSecond + tmpToEndSecond >= 60) {
      toEndSecond = initSecond + tmpToEndSecond - 60;
      tmpToEndMinute++;
    } else toEndSecond = initSecond + tmpToEndSecond;

    if (initMinute + tmpToEndMinute >= 60) {
      toEndMinute =  initMinute + tmpToEndMinute - 60;
      tmpToEndHour++;
    } else toEndMinute = initMinute + tmpToEndMinute;

    if (initHour + tmpToEndHour >= 24) {
      toEndHour = initHour + tmpToEndHour - 24;
    } else toEndHour = initHour + tmpToEndHour;

//  Serial.print("Misja zakonczy sie o : ");                                                      TODO
//  Serial.print(toEndHour); Serial.print(":");
//  Serial.print(toEndMinute); Serial.print(":");
//  Serial.print(toEndSecond); Serial.println();
    missionStart = false;
  }

  if(isMissionDone()){
      Serial.println("UpÃ…â€šynÃ„â€¦Ã…â€š czas misji.");  
      beginM = 0;  
      missionStart = true; 
      return;                      
  }

String actionValue = "0";

  bool isNotObstacle = digitalRead(proximitySensorInputPin);
  if(isNotObstacle == false){
     actionValue = "1";
    }
  else if(voltmeterCheck() < 5 && voltmeterCheck() > 0.01){
      actionValue = "2";
    }
   else if(isMissionDone()){
      actionValue = "3";
    }
 

  bmp280.awaitMeasurement();

  float temperature;
  bmp280.getTemperature(temperature);

  float pascal;
  bmp280.getPressure(pascal);

  bmp280.triggerMeasurement();

  int brightness = analogRead(photoResistorPin);
  
  //data += "\n" + String(printTime() + "\n" + voltmeterCheck() + "\n" + pascal + "\n" + temperature + "\n" + brightness + "\n" + digitalRead(proximitySensorInputPin));
  data += actionValue + "#" + printTime() + "#" + String(voltmeterCheck()) + "#" + String(isNotObstacle) + "#" + String(pascal/100) + "#" + String(temperature) + "#" + String(brightness) + "\n";

  counter++;
  if(counter == sampleCount){
  sendCollectedData();
  counter = 0;
  }

  

  delay(samplingTime);
    }
}
void missionAction(int i){
    Serial.println("ACTION " + String(i));
  }

void sendCollectedData(){
    Serial.print(data);
    data = "";
  }

bool isMissionDone(){
    if(dt.second >= toEndSecond && dt.minute == toEndMinute && dt.hour == toEndHour)
      return true;
    return false;
  }

//float temperatureCheck() {
//  bmp280.awaitMeasurement();
//
//  float temperature;
//  bmp280.getTemperature(temperature);
//  bmp280.triggerMeasurement();
//
//  return temperature ;
//}

float voltmeterCheck() {
  float voltageValue = 0.0;
  float vout = 0.00;
  float vin = 0.00;
  float R1 = 10000.00; 
  float R2 = 1000.00; 

  voltageValue = analogRead(voltmeterPin);
  vout = (voltageValue * 5.00) / 1024.00;
  vin = vout / (R2 / (R1 + R2)); 
  
  return vin;
}

String printTime() {

int year = dt.year;
int month = dt.month;
int day = dt.day;
int hour = dt.hour;
int minute = dt.minute;
int second = dt.second;
String delimeter = "-";

return dt.year + delimeter + dt.month + delimeter + day + " " + hour + ":" + minute + ":" + second;
}
