#include "doser.h"
#include "utility.h"

Doser::Doser(AsyncWebServer *server, Database *_db){
    db = _db;
    _server = server;
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }else{
        spiffsMounted = true;
    }
    util = new Utility();
    //setDbVariables();
    float amtF = db->getFloat("Doser/Blue/Calibration/secBluPerMl");
    Serial.print("secBluMl from db is: ");
    Serial.println(amtF);
    setBluSecPerMl(amtF);
    amtF = db->getFloat("Doser/Green/Calibration/secGrnPerMl");
    Serial.print("secGrnMl from db is: ");
    Serial.println(amtF);
    setGrnSecPerMl(amtF);
    amtF = db->getFloat("Doser/Yellow/Calibration/secYelPerMl");
    Serial.print("secYelMl from db is: ");
    Serial.println(amtF);
    setYelSecPerMl(amtF);
    amtF = db->getFloat("Doser/Purple/Calibration/secPurPerMl");
    Serial.print("secPurMl from db is: ");
    Serial.println(amtF);
    setPurSecPerMl(amtF);
    int amt = db->getInt("Doser/Blue/Dosing/bluMl");
    setBluMl(amt);
    amt = db->getInt("Doser/Green/Dosing/grnMl");
    setGrnMl(amt);
    amt = db->getInt("Doser/Yellow/Dosing/yelMl");
    setYelMl(amt);
    amt = db->getInt("Doser/Purple/Dosing/purMl");
    setPurMl(amt);
}

int Doser::calibrate(String color, bool start){
    int retVal = 0;
    if(color == "red"){
        if(start){
            int now = millis();
            retVal = now - startTime;
            motor("blue",1);
            //return 0;
        }else{
            int now = millis();
            retVal = now - startTime;
            motor("blue",0);
             //return stopTime;
        }
    }else if(color == "green"){
        if(start){
            int now = millis();
            retVal = now - startTime;
            motor("green",1);
            //return 0;
        }else{
            int now = millis();
            retVal = now - startTime;
            motor("green",0);
             //return stopTime;
        }
    }else if(color == "yellow"){
        if(start){
            int now = millis();
            retVal = now - startTime;
            motor("yellow",1);
            //return 0;
        }else{
           int now = millis();
            retVal = now - startTime;
            motor("yellow",0);
             //return stopTime;
        }
    }else if(color == "purple"){
        if(start){
            int now = millis();
            retVal = now - startTime;
            motor("purple",1);
            //return 0;
        }else{
            int now = millis();
            retVal = now - startTime;
            motor("purple",0);
             //return stopTime;
        }
    }
    //Serial.print("retVal is: ");
    Serial.println(retVal);
    return retVal/1000;
}

bool Doser::dose(String color){
    bool retVal = true;
    if(firstDoseTime){
        doseRun = 0; //TODO
        firstDoseTime = false;
        if(color == "blue"){
            doseRun = getBluSecPerMl()*getBluMl();
        }else if(color == "green"){
            doseRun = getGrnSecPerMl()*getGrnMl();
        }else if(color == "yellow"){
            doseRun = getYelSecPerMl()*getYelMl();
        }else if(color == "purple"){
            doseRun = getPurSecPerMl()*getPurMl();
        }
        //Serial.print("DoseRun is: ");
        //Serial.println(doseRun);
        firstDose = millis();
        motor(color,1);
    }
    unsigned long now = millis();
    unsigned long elapse = now - firstDose;
    //Serial.print("Elapse is: ");
    //Serial.println(elapse);
    if( elapse/1000 > doseRun ){
        motor(color, 0);
        Serial.print("Dosing ");
        Serial.print(color);
        Serial.print(" for this long: ");
        Serial.println(doseRun);
        firstDoseTime = true;
        retVal = false;
        elapse = 0;
        lastDoseRun = doseRun;
        //delay(100);
    }
    //if it runs TOO long shut them all down
    if(elapse > MAX_DOSE_RUN){
        motor("blue",0);
        motor("green",0);
        motor("yellow",0);
        motor("purple",0);
        //TODO once have a decent logger, log dosing ran too long
        retVal = false;
    }
    return retVal;
}

void Doser::motor(String pump, int value) {
  bool pumpRunning = false;
  if (pump == "blue") {
    Serial.print("Blue motor value is: ");
    Serial.println(value);
    digitalWrite(BLUEMOTOR, value);
  } else if (pump == "green") {
    Serial.print("Green motor value is: ");
    Serial.println(value);
    digitalWrite(GREENMOTOR, value);
  } else if (pump == "yellow") {
    Serial.print("Yellow motor value is: ");
    Serial.println(value);
    digitalWrite(YELLOWMOTOR, value);
  } else if (pump == "purple") {
    Serial.print("Purpley motor value is: ");
    Serial.println(value);
    digitalWrite(PURPLEMOTOR, value);
  }
}


bool Doser::setDbVariables(){
    bool retVal = true;
    db->putFloat("/Doser/Blue/Calibration/secBluPerMl",0.47);
    secBluPerMl = db->getFloat("/Doser/Blue/Calibration/secBluPerMl");
    String secBluPerMlStr = String(secBluPerMl);
    Serial.print("Blue cal is: ");
    Serial.println(secBluPerMlStr);
    db->putInt("/Doser/Blue/Dosing/bluMl",50);
    bluMl = db->getInt("/Doser/Blue/Dosing/bluMl");
    String bluMlStr = String(bluMl);
    util->writeFile(SPIFFS, "/secBluPerMl.txt", secBluPerMlStr.c_str());
    util->writeFile(SPIFFS, "/bluMl.txt", bluMlStr.c_str());
    db->putFloat("/Doser/Green/Calibration/secGrnPerMl",0.48);
    secGrnPerMl = db->getFloat("/Doser/Green/Calibration/secGrnPerMl");
    String secGrnPerMlStr = String (secGrnPerMl);
    db->putInt("/Doser/Green/Dosing/grnMl",51);
    grnMl = db->getInt("/Doser/Green/Dosing/grnMl");
    String grnMlStr = String(grnMl);
    util->writeFile(SPIFFS, "/secGrnPerMl.txt", secBluPerMlStr.c_str());
    util->writeFile(SPIFFS, "/grnMl.txt", bluMlStr.c_str());
    db->putFloat("/Doser/Yellow/Calibration/secYelPerMl",0.49);
    secYelPerMl = db->getFloat("/Doser/Yellow/Calibration/secYelPerMl");
    String secYelPerMlStr = String (secYelPerMl);
    db->putInt("/Doser/Yellow/Dosing/yelMl",52);
    yelMl = db->getInt("/Doser/Yellow/Dosing/yelMl");
    String yelMlStr = String(yelMl);
    util->writeFile(SPIFFS, "/secYelPerMl.txt", secYelPerMlStr.c_str());
    util->writeFile(SPIFFS, "/yelMl.txt", yelMlStr.c_str());
    db->putFloat("/Doser/Purple/Calibration/secPurPerMl",0.5);
    secPurPerMl = db->getFloat("/Doser/Purple/Calibration/secPurPerMl");
    String secPurPerMlStr = String (secPurPerMl);
    db->putInt("/Doser/Purple/Dosing/purMl",53);
    purMl = db->getInt("/Doser/Purple/Dosing/purMl");
    String purMlStr = String(purMl);
    util->writeFile(SPIFFS, "/secPurPerMl.txt", secPurPerMlStr.c_str());
    util->writeFile(SPIFFS, "/purMl.txt", purMlStr.c_str());
    return retVal;
}



float Doser::getBluSecPerMl(){
    return secBluPerMl;
}

void Doser::setBluSecPerMl(float secPerMl){
    secBluPerMl = secPerMl;
    String secBluPerMlStr = String(secBluPerMl);
    util->writeFile(SPIFFS, "/secBluPerMl.txt", secBluPerMlStr.c_str());
    Serial.print("secBlueMl is: ");
    Serial.println(secPerMl);
    db->putFloat("Doser/Blue/Calibration/secBluPerMl", secPerMl);
}

int Doser::getBluMl(){
    String bluMlStr =util->readFile(SPIFFS, "/bluMl.txt");
    return bluMlStr.toInt();
}

void Doser::setBluMl(int blMl){
    String bluMlStr = String(blMl);
    int bluMlInt = bluMlStr.toInt();
    util->writeFile(SPIFFS, "/bluMl.txt", bluMlStr.c_str());
    db->putInt("Doser/Blue/Dosing/bluMl", bluMlInt);

}

float Doser::getGrnSecPerMl(){
    return secGrnPerMl;
}

void Doser::setGrnSecPerMl(float secPerMl){
    secGrnPerMl = secPerMl;
    String secGrnPerMlStr = String(secGrnPerMl);
    util->writeFile(SPIFFS, "/secGrnPerMl.txt", secGrnPerMlStr.c_str());
    db->putFloat("Doser/Green/Calibration/secGrnPerMl", secPerMl);
}

int Doser::getGrnMl(){
    String grnMlStr =util->readFile(SPIFFS, "/grnMl.txt");
    return grnMlStr.toInt();
}

void Doser::setGrnMl(int grMl){
    String grnMlStr = String(grMl);
    int grnMlInt = grnMlStr.toInt();
    util->writeFile(SPIFFS, "/grnMl.txt", grnMlStr.c_str());
    db->putInt("Doser/Green/Dosing/grnMl", grnMlInt);

}

float Doser::getYelSecPerMl(){
    return secYelPerMl;
}

void Doser::setYelSecPerMl(float secPerMl){
    secYelPerMl = secPerMl;
    String secYelPerMlStr = String(secYelPerMl);
    util->writeFile(SPIFFS, "/secYelPerMl.txt", secYelPerMlStr.c_str());
    db->putFloat("Doser/Yellow/Calibration/secYelPerMl", secPerMl);
}

int Doser::getYelMl(){
    String yelMlStr =util->readFile(SPIFFS, "/yelMl.txt");
    return yelMlStr.toInt();
}

void Doser::setYelMl(int ywMl){
    String yelMlStr = String(ywMl);
    int yelMlInt = yelMlStr.toInt();
    util->writeFile(SPIFFS, "/yelMl.txt", yelMlStr.c_str());
    db->putInt("Doser/Yellow/Dosing/yelMl", yelMlInt);

}

float Doser::getPurSecPerMl(){
    return secPurPerMl;
}

void Doser::setPurSecPerMl(float secPerMl){
    secPurPerMl = secPerMl;
    String secPurPerMlStr = String(secPurPerMl);
    util->writeFile(SPIFFS, "/secPurPerMl.txt", secPurPerMlStr.c_str());
    db->putFloat("Doser/Purple/Calibration/secPurPerMl", secPerMl);
}

int Doser::getPurMl(){
    String purMlStr =util->readFile(SPIFFS, "/purMl.txt");
    return purMlStr.toInt();
}

void Doser::setPurMl(int puMl){
    String purMlStr = String(puMl);
    int purMlInt = purMlStr.toInt();
    util->writeFile(SPIFFS, "/purMl.txt", purMlStr.c_str());
    db->putInt("Doser/Purple/Dosing/purMl", purMlInt);

}



int Doser::getErrorCode(){
    return errorCode;
}
void Doser::setErrorCode(int errCode){
    errorCode = errCode;
}

int Doser::getDoseRun(){
    return lastDoseRun;
}