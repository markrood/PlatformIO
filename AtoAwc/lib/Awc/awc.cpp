#include "awc.h"

Awc::Awc(){

}

bool Awc::doAwc(){
    unsigned long now = 0;
    if(awcRunning){
        /////////////////////////////////// emptying ///////////////////////////////
        if(emptyFirstTime && !awcRunning){
            emptyFirstTime = false;
            now = millis();
            emptyRunning = true;
            awcRunning = true;
        }
        if(emptyRunning){
            digitalWrite(EMPTYPUMP, 1);
            if(digitalRead(LOWWATER)){     // 1 is out of water
                digitalWrite(EMPTYPUMP, 0);
                elapse = millis() - now;
                emptyRunning = false;
                emptyFirstTime = true;
                setEmptyRunTime(elapse);
            }
            if(elapse >= MAXEMPTYTIME){
                digitalWrite(EMPTYPUMP, 0);
                Serial.println("Empty ran too long");  //TODO set error code somewhere
                awcRunning = false;
            }
        }
        ////////////////////////////// filling ////////////////////////////////////
        if(fillFirstTime && !emptyRunning){
            fillFirstTime - false;
            now = millis();
            fillRunning = true;
        }
        if(fillRunning){
            digitalWrite(NEWPUMP, 1);
            if(!digitalRead(NORMALWATER)){   //1 is out of water
                digitalWrite(NEWPUMP, 0);
                elapse = millis() - now;
                fillRunning = false;
                fillFirstTime = true;
                awcRunning = false;
                setFillRunTime(elapse);
            }
            if(elapse >= MAXFILLTIME){
                digitalWrite(NEWPUMP, 0);
                Serial.println("Fill ran too long"); //TODO set error code somewhere
                awcRunning = false;
            }
        }
    }
    return awcRunning;
}

void Awc::setEmptyRunTime(unsigned long val){
    emptyRunTime = val;
}
int Awc::getEmptyRunTime(){
    return emptyRunTime/1000;
}
void Awc::setFillRunTime(unsigned long val){
    fillRunTime = val;
}
int Awc::getFillRunTime(){
    return fillRunTime/1000;
}



