#include "ato.h"

#include "Arduino.h"
#include "fishScheduler.h"
#include "ato.h"


//Ato::Ato():FishSched(){
Ato::Ato(){
    pinMode(NORMALWATER, INPUT);



    pinMode(ROPUMP, OUTPUT);
    digitalWrite(ROPUMP, 0);
 
}

bool Ato::doAto(){
    unsigned long now = 0;
    if(digitalRead(NORMALWATER)){// 1 is out of water
        if(atoFirstTime){
            atoRunning = true;
            now = millis();
            atoFirstTime = false;
        }
    digitalWrite(ROPUMP, 1);
    }else{
        digitalWrite(ROPUMP, 0);
        atoRunning = false;
        atoFirstTime = true;
        elapse = millis() - now;
        setTopOffRunTime(elapse);
    }
    if(elapse >= MAXTOPOFFTIME){
        digitalWrite(ROPUMP, 0);
        atoRunning = false;
        Serial.println("Top off ran too long");//TODO make an error code to db
    }
    return atoRunning;
}



        void Ato::setTopOffRunTime(unsigned long val){
            topOffRunTime = val;
        }
        int Ato::getTopOffRunTime(){
            return topOffRunTime/1000;
        }

