/**********************************************************************/
//
//  Class:
//    FishTime
//
//  Description
//    Super class for FishSched.
//    This is just a time class uses Real Time Clock (RTC) built into the esp32 and
//    updates RTC to correct time form the internet NTP time.
//
//  ERROR CODES:
//    none yet
//
/**************************************************************************/


#include "fishTime.h"
#include "time_zones.h"

FishTime::FishTime(){
  //Serial..println("FishTime constructor was called");
  //configTimeWithTz(getTzByLocation(location), ntpServer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    initTime();
}

void FishTime::initTime(){ 
  int i = 0;
//Serial.println("4.0");
  while(!getLocalTime(&timeinfo) && i < 5){
    Serial.println("Failed to obtain time in init");
    i++;
    delay(1000);
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  i = 0;
}

bool FishTime::update(){
    struct tm timeinfo;
    int i = 0;
    bool retVal = true;
//Serial.println("6.0");
  while(!getLocalTime(&timeinfo) && i<5){
    Serial.println("Failed to obtain time in update");
    retVal = false;
    i++;
    delay(1000);
  }
  i = 0;
  if(timeinfo.tm_mday == currentHour){
    //Serial.println("Still Current Day!");
    return retVal;
  }else{
    if(currentHour == 99){
      currentHour = timeinfo.tm_mday;
      //Serial.println("Frist time setting Day!");
      return retVal;
    }else{
      //do time update
      //configTimeWithTz(getTzByLocation(location), ntpServer);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      Serial.println("Just updated RTC with Network Time");
      currentHour = timeinfo.tm_mday;
      return retVal;
    }
  }
  return retVal;
}

int FishTime::getHour(){
  //Serial.println("IN GET HOUR");
  int hour = timeinfo.tm_hour;
  return timeinfo.tm_hour;
}

int FishTime::getMinute(){
  return timeinfo.tm_min;
}

int FishTime::getYear(){
  return timeinfo.tm_year;
}
int FishTime::getMonth(){
  return timeinfo.tm_mon;
}
int FishTime::getDay(){
  return timeinfo.tm_mday;
}

void FishTime::syncTime(){
  //configTimeWithTz(getTzByLocation(location), ntpServer); 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
