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
  initTime();
  Serial.println("FishTime constructor was called");
  configTimeWithTz(getTzByLocation(location), ntpServer);
}

void FishTime::initTime(){
    
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
}

bool FishTime::update(){
    struct tm timeinfo;
    bool retVal = true;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }
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
      configTimeWithTz(getTzByLocation(location), ntpServer);
      Serial.println("Just updated RTC with Network Time");
      currentHour = timeinfo.tm_mday;
      return retVal;
    }
  }
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
