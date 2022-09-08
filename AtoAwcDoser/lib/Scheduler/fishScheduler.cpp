/*****************************************************************************************/
//
//  Class:
//    FishSched
//
//  Description
//    User super class FishTime for RTC and NTP update time from internet.
//    This class used the RTC to call the "crons" It simple set flags in an
//    bitset/enum (flags) when a certain time is hit (i.e. hour,15min,30min,2h,
//    3h,4h,8h,12h,1am-noon, 1pm-midnight).  A main.cpp class will use this scheduler 
//    to look for events and act on the event and reset the flag for that event when fired
//
//  ERROR_CODES
//    none yet
//
/********************************************************************************************/

#include "fishScheduler.h"
#include "fishTime.h"



  FishSched::FishSched():FishTime(){
    initTime();
    update();
  }
void FishSched::updateMyTime(){
    update();
}

void FishSched::tick(){
//Serial.println("1.11");
  int nowHour = getHour();
    if(nowHour != currHour ){
      if(firstHour){
        firstHour = false;
        currHour = nowHour;
        return;
      }
    currHour = nowHour;
    //Serial.print("New Hour is: ");
    //Serial.println(currHour);
    blowHourHorn(currHour);
    }
}

void FishSched::tock(){
//Serial.println("1.22");
  int nowMinute = getMinute();
  //Serial.println("1.23");
  if(nowMinute != currentMinute){
    //Serial.println("1.24");
    currentMinute = nowMinute;
    ////Serial.println(currentMinute);
    blow15MinuteHorn(currentMinute);
    //Serial.println("1.25");
    blow30MinuteHorn(currentMinute);
  }
}

void FishSched::blowHourHorn(int hr){
  //Serial.println("0.1111");
  addHour();
  if(hr == 1){
    //Serial.println("the hour is 1");
    flags.set(OneAm);
  }else if(hr == 2){
    //Serial.println("the hour is 2"); 
    flags.set(TwoAm);
  }else if(hr == 3){
    //Serial.println("the hour is 3");  
    flags.set(ThreeAm);
  }else if(hr == 4){
    //Serial.println("the hour is 4");
    flags.set(FourAm);  
  }else if(hr == 5){
    //Serial.println("the hour is 5"); 
    flags.set(FiveAm); 
  }else if(hr == 6){
    //Serial.println("the hour is 6"); 
    flags.set(SixAm); 
  }else if(hr == 7){
    //Serial.println("the hour is 7"); 
    flags.set(SevenAm); 
  }else if(hr == 8){
    //Serial.println("the hour is 8"); 
    flags.set(EightAm); 
  }else if(hr == 9){
    //Serial.println("the hour is 9");  
    flags.set(NineAm);
  }else if(hr == 10){
    //Serial.println("the hour is 10");
    flags.set(TenAm);  
  }else if(hr == 11){
    //Serial.println("the hour is 11");
    flags.set(ElevenAm);  
  }else if(hr == 12){
    //Serial.println("the hour is 12"); 
    flags.set(Noon); 
  }else if(hr == 13){
    //Serial.println("the hour is 13"); 
    flags.set(OnePm);
  }else if(hr == 14){
    //Serial.println("the hour is 14"); 
    flags.set(TwoPm); 
  }else if(hr == 15){
    //Serial.println("the hour is 15"); 
    flags.set(ThreePm); 
  }else if(hr == 16){
    //Serial.println("the hour is 16");  
    flags.set(FourPm);
  }else if(hr == 17){
    //Serial.println("the hour is 17"); 
    flags.set(FivePm);
  }else if(hr == 18){
    //Serial.println("the hour is 18");
    flags.set(SixPm);  
  }else if(hr == 19){
    //Serial.println("the hour is 19");
    flags.set(SevenPm);  
  }else if(hr == 20){
    //Serial.println("the hour is 20"); 
    flags.set(EightPm); 
  }else if(hr == 21){
    //Serial.println("the hour is 21");
    flags.set(NinePm);
  }else if(hr == 22){
    //Serial.println("the hour is 22"); 
    flags.set(TenPm); 
  }else if(hr == 23){
    //Serial.println("the hour is 23");  
    flags.set(ElevenPm);
  }else if(hr == 0){
    //Serial.println("the hour is 0");  
    flags.set(Midnight);
  }
  //Serial.println("doneeeeeeeee with flags.set");

}

void FishSched::blow15MinuteHorn(int m){
  if(m == 15 || m == 30 || m == 45 || m == 0){
    //Serial.println("Fifteen Minute Alarm");
    flags.set(Fifteen);
    //Serial.println("Fifteen Minute Alarm");
  }
}

void FishSched::blow30MinuteHorn(int m){
  if(m == 30 || m == 0){
    //Serial.println("Thirty Minute Alarm");
    flags.set(Thirty);
    //Serial.println("Thirty Minute Alarm");
  }
}

void FishSched::addHour(){
  oneHourAlarm++;
  twoHourAlarm++;
  threeHourAlarm++;
  fourHourAlarm++;
  eightHourAlarm++;
  twelveHourAlarm++;
//Serial.println("3.0");

  if(oneHourAlarm == 1){
    oneHourAlarm = 0;
    //blow2HourHorn();
    flags.set(Hour);
    Serial.println("One Hour Alarm");
  }
  if(twoHourAlarm == 2){
    twoHourAlarm = 0;
    //blow2HourHorn();
    flags.set(TwoHour);
    Serial.println("Two Hour Alarm");
  }
  if(threeHourAlarm == 3){
    threeHourAlarm = 0;
    //blow3HourHorn();
    flags.set(ThreeHour);
    Serial.println("Three Hour Alarm");
  }

  if(fourHourAlarm == 4){
    fourHourAlarm = 0;
    //blow4HourHorn();
    Serial.println("Four Hour Alarm");
    flags.set(FourHour);
  }

  if(eightHourAlarm == 8){
    eightHourAlarm = 0;
    //blow8HourHorn();
    flags.set(EightHour);
    Serial.println("Eight Hour Alarm");
  }

  if(twelveHourAlarm == 12){
    twelveHourAlarm = 0;
    //blow12HourHorn();
    flags.set(TwelveHour);
    Serial.println("Twelve Hour Alarm");
  }

}
/*template<size_t N>
std::bitset<N> FishSched::getFlags(){
  return flags;
}*/
bool FishSched::isFlagSet(int flag){
  bool retVal = flags.test(flag);
  //flags.flip(Fifteen);
  return retVal;
}

void FishSched::resetFlag(int flag){
 // Serial.println("2,2,2");
  flags.flip(flag);
  //Serial.println("2.2.4");
}

