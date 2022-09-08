/***********************************************************************/
//  Description:
//    This main class just loops and calls tick and tock to run the
//    event machine in FishScheduler.  The if loops thru all 31 events
//    (i.e. hour,15min,30min,2h,3h,4h,8h,12h,1am-noon, 1pm-midnight) to see
//    if that event is set.  If one of these events is set It will do what 
//    is scheduled to run on this event and then reset the event.  In this 
//    case it will dose blue, green, yellow, or purple on a particular event.
//
//    This main class also runs the Dose calibration.  It will read a button
//    pressed to start the calibration and read the subsequent button push
//    to stop the calibration.  Dose class has the calibration funtion...
//    so this main in call dose->calibrate.
//
// Classes: 
//    Doser
//
//      Methods
//        dose(String color)
//        calibrate(String color,bool start)
//
//  Scheduler
//    Methods
//      updateMyTime()
//      initTime()
//      tick()   --does hour
//      tock()   --does minute
//      isFlagSet()  -- did a cron go off and which one (1-31)
//      resetFlag()  -- turns cron indicator off
//      
// ERROR CODES
//  M_wfnc   -- wifi is not connected
//  D_B_tl   -- Dosed Blue ran too long...shut dosing down
//  D_G_tl   -- Dosed Green ran too long...shut dosing down
//  D_Y_tl   -- Dosed Yellow ran too long...shut dosing down
//  D_P_tl   -- Dosed Purple ran too long...shut dosing down 
//  M_ftgt   -- failed to get time from internet
//
//  Pins
//    BLUEMOTOR = 5;
//    GREENMOTOR = 33;
//    YELLOWMOTOR = 32;
//    PURPLEMOTOR = 15;
//    BLU_BTN_PIN = 26;  TODO get real values for cal buttons
//    GRN_BTN_PIN =27;
//    YW_BTN_PIN = 14;
///   PU_BTN_PIN = 12;
//
/***********************************************************************/


#include <Arduino.h>
#include "AsyncTCP.h"

#include "ESPAsyncWebServer.h"
#include "ESPAsyncWiFiManager.h"         //https://github.com/tzapu/WiFiManager
//#include <WiFiManager.h>
#include <WebSerial.h>
#include <ESPmDNS.h>
#include "fishScheduler.h"
#include "button.h"
#include "doser.h"
#include "fbdb.h"
#include "utility.h"
//#include "SPIFFS.h"
#include "Effortless_SPIFFS.h"


AsyncWebServer server(80);
DNSServer dns;
void configModeCallback (AsyncWiFiManager *myWiFiManager);
int addDailyDoseAmt(int color,int val);
bool writeDailyDoseAmtToDb(String color, int amt);
//void startSpiffs();
//String readFile(fs::FS &fs, const char * path);
//void writeFile(fs::FS &fs, const char * path, const char * message);
bool writeDailyDosesToDb();
bool writeCalibrationToDb(int color, float amt);
void setDate();


int led = 2;
FishSched *mySched;
Doser *doser;
Database *db;
Utility *util;
//FishTime *myTime;
    enum{Hour,Fifteen,Thirty,TwoHour,ThreeHour,FourHour,EightHour,TwelveHour,
        Midnight,OneAm,TwoAm,ThreeAm,FourAm,FiveAm,SixAm,SevenAm,EightAm,NineAm,TenAm,ElevenAm,
        Noon,OnePm,TwoPm,ThreePm,FourPm,FivePm,SixPm,SevenPm,EightPm,NinePm,TenPm,ElevenPm,
        NumFlags
        };

//calibration buttons
Button blueBtn;
Button greenBtn;
Button yellowBtn;
Button purpleBtn;
//TODO set these to the correct hardware pins for calibration pins/buttons
int BLU_BTN_PIN = 19;
int GRN_BTN_PIN = 18;
int YW_BTN_PIN = 23;
int PU_BTN_PIN = 17;
int BLUEMOTOR = 33;
int GREENMOTOR = 26;
int YELLOWMOTOR = 14;
int PURPLEMOTOR = 13;
int LED = 2;

int MINCALTIME = 10;

bool calibrationRunning = false;
bool blueDosing = false;
bool greenDosing = false;
bool yellowDosing = false;
bool purpleDosing = false;
bool bluCalRunning = false;
bool grnCalRunning = false;
bool ywCalRunning = false;
bool puCalRunning= false;

//TODO button test junk remove
long bluBtnTime = 0;
long bluBtnMax = 49000;
bool bluBtnFirst = true;
bool bluePushed = false;
bool greenPushed = false;
bool yellowPushed = false;
bool purplePushed = false;
bool printedStart = false;
bool printedStop = false;
bool blueWasPressed = false;
bool greenWasPressed = false;
bool yellowWasPressed = false;
bool purpleWasPressed = false;
int buttonClrCnt = 0;

int blueDailyDoseAmt = 0;
int greenDailyDoseAmt = 0;
int yellowDailyDoseAmt = 0;
int purpleDailyDoseAmt = 0;
String blueDailyDoseAmtStr = "";
String greenDailyDoseAmtStr = "";
String yellowDailyDoseAmtStr = "";
String purpleDailyDoseAmtStr = "";
  int x = 0;
  int w = 0;
  bool blinking = false;

eSPIFFS fileSystem;

 int yr = 0;
  String yrStr = "";
  int mo = 0;
  int da = 0;
  String daStr = "";
  

 


//TODO
//int testInt = 0; //just a test int for faking day REMOVE it


void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  Serial.begin(115200);
  //pinMode(led, OUTPUT);
  pinMode(BLU_BTN_PIN,INPUT_PULLUP);
  pinMode(GRN_BTN_PIN,INPUT_PULLUP);
  pinMode(YW_BTN_PIN,INPUT_PULLUP);
  pinMode(PU_BTN_PIN,INPUT_PULLUP);

  pinMode(BLUEMOTOR,OUTPUT);
  pinMode(GREENMOTOR,OUTPUT);
  pinMode(YELLOWMOTOR,OUTPUT);
  pinMode(PURPLEMOTOR,OUTPUT);
  pinMode(LED,OUTPUT);

    ///////////////////Start WiFi ///////////////////////////////////////
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server, &dns);
  //reset settings - for testing
  //wifiManager.resetSettings();
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,175), IPAddress(192,168,1,1), IPAddress(255,255,255,0), IPAddress(192,168,1,1), IPAddress(192,168,1,1));
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Doser")) {
    Serial.println("failed to connect and hit timeout");
    Serial.println("restarting esp");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }
  delay(50);
  //Serial.print("FreeHeap is :");
  //Serial.println(ESP.getFreeHeap());
  delay(50);
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  server.begin();
 WebSerial.begin(&server);
 delay(100);
  //WebSerial.print("local ip is: ");
  //WebSerial.println(WiFi.localIP());
  mySched = new FishSched();
  db  = new Database();
  db->initDb();
  doser = new Doser(&server,db);
  //util = new Utility;
   
  mySched->updateMyTime();
  mySched->initTime();
  //setup calibrate buttons
  blueBtn.begin(BLU_BTN_PIN);
  greenBtn.begin(GRN_BTN_PIN);
  yellowBtn.begin(YW_BTN_PIN);
  purpleBtn.begin(PU_BTN_PIN);
  //int da = 15;

   // Create a eSPIFFS class
  #ifndef USE_SERIAL_DEBUG_FOR_eSPIFFS
    // Create fileSystem
    

    // Check Flash Size - Always try to incorrperate a check when not debugging to know if you have set the SPIFFS correctly
    if (!fileSystem.checkFlashConfig()) {
      Serial.println("Flash size was not correct! Please check your SPIFFS config and try again");
      delay(100000);
      ESP.restart();
    }
  #else
    // Create fileSystem with debug output
    eSPIFFS fileSystem(&Serial);  // Optional - allow the methods to print debug
  #endif
  
  // Serial.begin(115200);

  //util->startSpiffs();
  String test = "test";
  //util->writeFile(SPIFFS,"/test", test.c_str());

  //db->initDb();
  setDate();
  delay(10);
}

void loop() {
//TODO remove just fashing led......
  if(blinking){
    digitalWrite(LED, 0);
    blinking = false;
  }else{
    digitalWrite(LED, 1);
    blinking = true;
  }
  //check if any calibraton button is pressed and only do that

  //Serial.println("0.0.0");
  bool bluePushed = blueBtn.debounce();
  //Serial.println("0.0.1");
  bool greenPushed = greenBtn.debounce();
  //Serial.println("0.0.2");
  bool yellowPushed = yellowBtn.debounce();
  //Serial.println("0.0.3");
  bool purplePushed = purpleBtn.debounce();
  //Serial.println("0.0.4");

   while (WiFi.status() != WL_CONNECTED ){
    if(w < 2){
      Serial.print('!');
      WebSerial.print("M_wfnc");
      delay(1000);
      w = 0;
      break;
    }else{
      w++;
    }
  }
  
  //Serial.println(WiFi.localIP());

  if(bluePushed && !grnCalRunning && !ywCalRunning && !puCalRunning){
    bluePushed = false;
//Serial.println("0.0");
    if(!bluCalRunning){
      bluCalRunning = true;
      calibrationRunning = true;
    }else{
      bluCalRunning = false;
    }
  }

  if(greenPushed && !bluCalRunning && !ywCalRunning && !puCalRunning){
    //Serial.println("?????????????HERE");
//Serial.println("0.001");
    greenPushed = false;
//Serial.println("0.1");
    if(!grnCalRunning){
      grnCalRunning = true;
      calibrationRunning = true;
    }else{
      grnCalRunning = false;
    }
  }

  
  if(yellowPushed && !grnCalRunning && !bluCalRunning && !puCalRunning){
    yellowPushed = false;
//Serial.println("0.3");
    if(!ywCalRunning){
      ywCalRunning = true;
      calibrationRunning = true;
    }else{
      ywCalRunning = false;
    }
  }
  
  if(purplePushed && !grnCalRunning && !ywCalRunning && !bluCalRunning){
    purplePushed = false;
//Serial.println("0.4");
    if(!puCalRunning){
      puCalRunning = true;
      calibrationRunning = true;
    }else{
      puCalRunning = false;
    }
  }
  
//TODO remove this button test
/*    if(bluBtnFirst){
      bluBtnTime = millis();
      bluBtnFirst = false;
      bluePushed = false;
    }
    long rightNow = millis();
    if(rightNow - bluBtnTime > bluBtnMax){
      bluBtnFirst = true;
      bluePushed = true;
      blueWasPressed = true;
      Serial.println("Blue Button Pressed");
    }*/
////////////////////////////////////////////////
///////////////////////////////////STOPPED HERE GREEN BUTTON NOTWORKING
 /*    if(bluBtnFirst){
      bluBtnTime = millis();
      bluBtnFirst = false;
      greenPushed = false;
     }
    long rightNow = millis();
    if(rightNow - bluBtnTime > bluBtnMax){
      bluBtnFirst = true;
      greenPushed = true;
      greenWasPressed = true;
      Serial.println("Green Button Pressed");
    }*/
////////////////////////////////////////////////
/*    if(bluBtnFirst){
      bluBtnTime = millis();
      bluBtnFirst = false;
      yellowPushed = false;
    }
    long rightNow = millis();
    if(rightNow - bluBtnTime > bluBtnMax){
      bluBtnFirst = true;
      yellowPushed = true;
      yellowWasPressed = true;
      Serial.println("Yellow Button Pressed");
    }*/
////////////////////////////////////////////////

/*    if(bluBtnFirst){
      bluBtnTime = millis();
      bluBtnFirst = false;
      purplePushed = false;
    }
    long rightNow = millis();
    if(rightNow - bluBtnTime > bluBtnMax){
      bluBtnFirst = true;
      purplePushed = true;
      purpleWasPressed = true;
      Serial.println("Purple Button Pressed");
      /*buttonClrCnt++;
      if(buttonClrCnt == 2){
        purplePushed = false;
        Serial.println("Yellow pressed");
        yellowPushed = true;
      }
      if(buttonClrCnt == 3){
        purplePushed = true;
        yellowPushed = false;
        buttonClrCnt = 0;
      }
    }*/
////////////////////////////////////////////////
 
  if(calibrationRunning){
//Serial.println("0.5");
    if(blueWasPressed){
      if(bluCalRunning && !grnCalRunning && !ywCalRunning && !puCalRunning){
        //start blue cal
        if(!printedStart){
          //Serial.println("Blue cal started");
          Serial.println("Blue cal started");
          doser->calibrate("blue", true);
          printedStart = true;
        }
      }else if(!grnCalRunning && !ywCalRunning && !puCalRunning && calibrationRunning){
        calibrationRunning = false;
        bluCalRunning = false;
        //stop blue cal
        //Serial.println("Blue cal stopped");
        Serial.println("Blue cal stopped");
        int time = doser->calibrate("blue", false);
        if(time > MINCALTIME){
          doser->setBluSecPerMl(time/100);
          bool inserted = writeCalibrationToDb(1, time/100);
        }
        Serial.println(time);
        printedStart = false;
        blueWasPressed = false;
      }
    }
    if(greenWasPressed){
      if(grnCalRunning && !bluCalRunning && !ywCalRunning && !puCalRunning){
        //start blue cal
        //Serial.println("****************i got here*************");
        if(!printedStart){
          //Serial.println("Green cal started");
          Serial.println("Green cal started");
          doser->calibrate("green", true);
          printedStart = true;
        }
      }else if (!bluCalRunning && !ywCalRunning && !puCalRunning && calibrationRunning){
        calibrationRunning = false;
        grnCalRunning = false;
        //stop blue cal
        //Serial.println("Green cal stopped");
        Serial.println("Green cal stopped");
        int time = doser->calibrate("green", false);
        if(time > MINCALTIME){
          doser->setGrnSecPerMl(time/100);
          bool inserted = writeCalibrationToDb(2, time/100);
        }
        Serial.println(time);
        printedStart = false;
        greenWasPressed = false;
      }
    }
    if(yellowWasPressed){
      if(ywCalRunning && !bluCalRunning && !grnCalRunning && !puCalRunning){
        //start blue cal
        if(!printedStart){
          //Serial.println("Yellow cal started");
          Serial.println("Yellow cal started");
          doser->calibrate("yellow", true);
          printedStart = true;
        }
      }else if(!bluCalRunning && !grnCalRunning && !puCalRunning && calibrationRunning){
        calibrationRunning = false;
        ywCalRunning = false;
        //stop blue cal
        //Serial.println("Yellow cal stopped");
        Serial.println("Yellow cal stopped");
        int time = doser->calibrate("yellow", false);
        if(time > MINCALTIME){
          doser->setYelSecPerMl(time/100);
        bool inserted = writeCalibrationToDb(3, time/100);
        }
      Serial.println(time);
        //TODO save mlPerSec to SPIFFS!!!!!!!!!!!!!!
        //Serial.print("purple mlPerSec is: ");
        //Serial.println(mlPerSec);
        printedStart = false;
        yellowWasPressed = false;
      }
    }
    if(purpleWasPressed){
      if(puCalRunning && !grnCalRunning && !ywCalRunning && !bluCalRunning){
        //start blue cal
        if(!printedStart){
          //Serial.println("Purple cal started");
          Serial.println("Purple cal started");
          doser->calibrate("purple", true);
          printedStart = true;
        }
      }else if(!grnCalRunning && !ywCalRunning && !bluCalRunning && calibrationRunning){
        calibrationRunning = false;
        puCalRunning = false;
        //stop blue cal
        //Serial.println("Purple cal stopped");
        Serial.println("Purple cal stopped");
        int time = doser->calibrate("purple", false);
        if(time > MINCALTIME){
          doser->setPurSecPerMl(time/100);
          bool inserted = writeCalibrationToDb(4, time/100);
        }
        //TODO save mlPerSec to SPIFFS!!!!!!!!!!!!!!
        //Serial.print("purple mlPerSec is: ");
        //Serial.println(mlPerSec);
        printedStart = false;
        purpleWasPressed = false;
      }
    }
  }else{
//Serial.println("1.0");
    mySched->initTime(); //initializes time MUST DO THIS
  // mySched->updateMyTime();
    mySched->tick(); //sets hour
    //Serial.print("Hour is: ");
    //Serial.println(nowHour);

    mySched->tock();//sets minute
    //Serial.print("Minute is: ");
    //Serial.println(nowMinute);
  
    //std::bitset<25> flags = mySched->getFlags();

    //TODO remove this /////////////////
  // int randNumber = random(31);
  // mySched->resetFlag(randNumber);
  /////////////////////////////////////

//Serial.println("2.0");
int z = 0;
    for(int i= 0;i<NumFlags-1;i++){
      bool flagSet = mySched->isFlagSet(i);
z++;
//Serial.print("z is: ");
//Serial.println(z);
      if(flagSet){
        //Serial.print("i is: ");
        //WebSerial.println("i is: ");
        //Serial.println(i);
        //WebSerial.println(i);
        //Serial.print("Value is: ");
        // WebSerial.println("Value is: ");
       //Serial.println(flagSet);
        //WebSerial.println(flagSet);
        
        if(i== 0){
          ////Serial.println("One Hour");
          if(!blueDosing && !greenDosing && !purpleDosing){
              WebSerial.print("Y");
            yellowDosing = doser->dose(3);
            if(!yellowDosing){         
              mySched->resetFlag(0);
              Serial.println("y");
              WebSerial.print("y"); 
              int err = doser->getErrorCode();
              if(err == 99){
                WebSerial.println("D_Y_tl");
              }
              int amt = addDailyDoseAmt(3,doser->getDoseRun());
            delay(1000); // have to delay here so doesn't reset 15 min timer wait a minute
            }
          }
        }
        if(i == 1){

          //Serial.println("Fifteen Minutes");
            //Serial.print("@");
            //Serial.print("************************  ");
            //Serial.println(ESP.getFreeHeap());
//Serial.println("2.1");
          if(!greenDosing && !yellowDosing && !purpleDosing){
            WebSerial.print("B");
            blueDosing = doser->dose(1);
            if(!blueDosing){         
//Serial.println("2.1");
              mySched->resetFlag(1);
              Serial.println("b");
              WebSerial.print("b");
              
              int err = doser->getErrorCode();
              if(err == 99){
                WebSerial.println("D_B_tl");
              }
              int amt = addDailyDoseAmt(1,doser->getDoseRun());
              //TODO remove
             
              //mySched->resetFlag(1);
              delay(1000); // have to delay here so doesn't reset 15 min timer wait a minute
            }
          }
                    //TODO remove this it is at midnight just testing a every 15min
        }
        if(i== 2){
          //Serial.println("Thirty Minutes");
          if(!blueDosing && !yellowDosing && !purpleDosing){
              WebSerial.print("G");
              greenDosing = doser->dose(2);
              //Serial.println("Shi9");
            if(!greenDosing){         
              mySched->resetFlag(2);
              Serial.println("g");
              WebSerial.print("g");
               int err = doser->getErrorCode();
              if(err == 99){
                WebSerial.println("D_G_tl");
              }
              int amt = addDailyDoseAmt(2,doser->getDoseRun());
           delay(1000); // have to delay here so doesn't reset 15 min timer wait a minute
            }
          }
        }
        if(i== 3){
          //Serial.println("Two Hour");
          if(!blueDosing && !yellowDosing && !greenDosing){
              WebSerial.print("P");
            purpleDosing = doser->dose(4);
            if(!purpleDosing){         
              mySched->resetFlag(3);
              Serial.println("p");
              WebSerial.print("p");
              int err = doser->getErrorCode();
              if(err == 99){
                WebSerial.println("D_P_tl");
              }
              int amt = addDailyDoseAmt(4,doser->getDoseRun());
            delay(1000); // have to delay here so doesn't reset 15 min timer wait a minute
            }
          }
        }
        if(i== 4){
          Serial.println("Three Hour");
              mySched->resetFlag(4);
        }
        if(i== 5){
          Serial.println("Four Hour");
          mySched->syncTime();
          mySched->resetFlag(5);
        }
        if(i== 6){
          Serial.println("Eight Hour");
          mySched->resetFlag(6);
        }
        if(i== 7){
           Serial.println("Twelve Hour");
            mySched->resetFlag(7);
        }
        if(i == 8){ 
          Serial.println("Midnight");
          bool didDbWrite = writeDailyDosesToDb();
          mySched->resetFlag(8);
        }
        if(i== 9){
          Serial.println("One Am");
          mySched->resetFlag(9);
        }
        if(i== 10){
         // Serial.println("Two Am");
          mySched->resetFlag(10);
        }
        if(i== 11){
          Serial.println("Three Am");
          mySched->resetFlag(11);
        }
        if(i== 12){
          Serial.println("Four Am");
          mySched->resetFlag(12);
        }
        if(i== 13){
         // Serial.println("Five Am");
          mySched->resetFlag(13);
        }
        if(i== 14){
          Serial.println("Six Am");
          mySched->resetFlag(14);
        }
        if(i== 15){
          Serial.println("Seven Am");
          mySched->resetFlag(15);
        }
        if(i== 16){
          Serial.println("Eight Am");
          mySched->resetFlag(16);
        }
        if(i==17){
          Serial.println("Nine Am");
          mySched->resetFlag(17);
        }
        if(i== 18){
          Serial.println("Ten Am");
          mySched->resetFlag(18);
        }
        if(i== 19){
          Serial.println("Eleven Am");
          mySched->resetFlag(19);
        }
        if(i== 20){ // doing time update an noon
          Serial.println("Noon");
          bool retVal = mySched->update();
          if(!retVal){
            WebSerial.print("M_ftgt");
          }
          mySched->resetFlag(20);
        }
        if(i== 21){
         // Serial.println("One Pm");
          mySched->resetFlag(21);
        }
        if(i== 22){
         // Serial.println("Two Pm");
          mySched->resetFlag(22);
        }
        if(i== 23){
          // Serial.println("Three Pm");
          mySched->resetFlag(23);
       }
        if(i== 24){
          Serial.println("Four Pm");
          mySched->resetFlag(24);
        }
        if(i== 25){
          Serial.println("Five Pm");
          mySched->resetFlag(25);
        }
        if(i== 26){
         // Serial.println("Six Pm");
          mySched->resetFlag(26);
        }
        if(i== 27){
          Serial.println("Seven Pm");
          mySched->resetFlag(27);
        }
        if(i==28){
           Serial.println("Eight Pm");
         mySched->resetFlag(28);
        }
        if(i== 29){
          Serial.println("Nine Pm");
          mySched->resetFlag(29);
        }
        if(i== 30){
          Serial.println("Ten Pm");
          mySched->resetFlag(30);
        }
        if(i== 31){
        Serial.println("Eleven Pm");
        setDate();
         mySched->resetFlag(31);
        }
      }
    }
    //Serial.println("Dam");
    delay(1000);
    //Serial.println(ESP.getFreeHeap());
    //WebSerial.println(ESP.getFreeHeap());
    
    //Serial.print(".");
    WebSerial.print(".");
   
    //TODO just testing button junk remove
    //if(!blueDosing && !greenDosing && !yellowDosing && !purpleDosing){
      //delay(45000);
    //}

  }//normal loop no calibraton

}

//////////////////////////////////////////////////////////////
// call back needed for wifi
/////////////////////////////////////////////////////////////
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  //myWiFiManager->startConfigPortal("ATOAWC");addDailyDoseAmt
  //myWiFiManager->autoConnect("DOSER");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}

int addDailyDoseAmt(int color,int val){
  int retVal =0;
//Serial.println("2.2");
  if(color == 1){
    //Serial.println("2.2.1");
    //blueDailyDoseAmtStr = readFile(SPIFFS, "/blueDailyDoseAmt.txt");
    fileSystem.openFromFile("/blueDailyDoseAmt.txt", blueDailyDoseAmt);
    //Serial.println("2.2.2");
    /*if(blueDailyDoseAmtStr != ""){
      blueDailyDoseAmt = blueDailyDoseAmtStr.toInt();
    }else{
      //TODO
      blueDailyDoseAmt = 0;
    }
    //Serial.println("2.2.3");*/
    blueDailyDoseAmt = blueDailyDoseAmt+val;
    //Serial.println("2.2.4");
    retVal = blueDailyDoseAmt;
    //Serial.println("2.2.5");
    //String blueDailyDoseAmtStrr = String(blueDailyDoseAmt);
    //Serial.println("2.2.6");
    delay(100);
    //util->writeFile(SPIFFS,"/blueDailyDoseAmt.txt", blueDailyDoseAmtStrr.c_str());
    fileSystem.saveToFile("/blueDailyDoseAmt.txt", blueDailyDoseAmt);
     //Serial.println("2.2.7");
   Serial.print("Blue Daily Dose Amt is: ");
    Serial.println(blueDailyDoseAmt);
    //blueDailyDoseAmtStr.clear();
    //blueDailyDoseAmtStrr.clear();
    delay(500);
  }else if(color == 2){
     //Serial.println("2.3.1");
    //greenDailyDoseAmtStr = readFile(SPIFFS, "/greenDailyDoseAmt.txt");
    fileSystem.openFromFile("/greenDailyDoseAmt.txt", greenDailyDoseAmt);
     //Serial.println("2.3.2");
     /*if(greenDailyDoseAmtStr != ""){
      greenDailyDoseAmt = greenDailyDoseAmtStr.toInt();
     }else{
      //TODO
      greenDailyDoseAmt = 0;
     }*/
     //Serial.println("2.3.3");
    greenDailyDoseAmt = greenDailyDoseAmt+val;
     //Serial.println("2.3.4");
    retVal = greenDailyDoseAmt;
     //Serial.println("2.3.5");
    //String geenDailyDoseAmtStrr = String(greenDailyDoseAmt);
       //Serial.println("2.3.6");
  delay(100);
    //util->writeFile(SPIFFS,"/greenDailyDoseAmt.txt", geenDailyDoseAmtStrr.c_str());
    fileSystem.saveToFile("/greenDailyDoseAmt.txt", greenDailyDoseAmt);
     //Serial.println("2.3.7");
     //greenDailyDoseAmtStr.clear();
    // geenDailyDoseAmtStrr.clear();
    delay(500);
  }else if(color == 3){
     //Serial.println("2.4.1");
    //yellowDailyDoseAmtStr = readFile(SPIFFS, "/yellowDailyDoseAmt.txt");
    fileSystem.openFromFile("/yellowDailyDoseAmt.txt", yellowDailyDoseAmt);
     //Serial.println("2.4.2");
     /*if(yellowDailyDoseAmtStr != ""){
      yellowDailyDoseAmt = yellowDailyDoseAmtStr.toInt();
     }else{
      //TODO
      yellowDailyDoseAmt = 0;
     }*/
      //Serial.println("2.4.3");
   yellowDailyDoseAmt = yellowDailyDoseAmt+val;
     //Serial.println("2.4.4");
    retVal = yellowDailyDoseAmt;
     //Serial.println("2.4.5");
    //String yellowDailyDoseAmtStrr = String(yellowDailyDoseAmt);
     //Serial.println("2.4.6");
    delay(100);
  
    //util->writeFile(SPIFFS,"/yellowDailyDoseAmt.txt", yellowDailyDoseAmtStrr.c_str());
    fileSystem.saveToFile("/yellowDailyDoseAmt.txt", yellowDailyDoseAmt);
     //Serial.println("2.4.7");
     //yellowDailyDoseAmtStr.clear();
     //yellowDailyDoseAmtStrr.clear();
    delay(500);
  }else if(color == 4){
    //Serial.println("10.0");
   // purpleDailyDoseAmtStr = readFile(SPIFFS, "/purpleDailyDoseAmt.txt");
   fileSystem.saveToFile("/purpleDailyDoseAmt.txt", purpleDailyDoseAmt);
    //Serial.println("10.1");
    //Serial.print("@@@@@@@@@@@@@@@@@@@@@@@@ spiff is ");
    /*Serial.println(purpleDailyDoseAmtStr);
    if(purpleDailyDoseAmtStr != ""){
      purpleDailyDoseAmt = purpleDailyDoseAmtStr.toInt();
    }else{
      //TODO something wrong
      Serial.println("ERROR no purpleDaily dose amount");
      purpleDailyDoseAmt = 0;
    }*/
    //Serial.println("10.2");
    purpleDailyDoseAmt = purpleDailyDoseAmt+val;
    //Serial.println("10.3");
    retVal = purpleDailyDoseAmt;
    //Serial.println("10.4");
    //String purpleDailyDoseAmtStrr = String(purpleDailyDoseAmt);
    //Serial.println("10.5");
    delay(100);
    //util->writeFile(SPIFFS,"/purpleDailyDoseAmt.txt", purpleDailyDoseAmtStrr.c_str());
    fileSystem.saveToFile("/purpleDailyDoseAmt.txt", purpleDailyDoseAmt);
    //purpleDailyDoseAmtStr.clear();
    //purpleDailyDoseAmtStrr.clear();
    delay(500);
  }
  return retVal;
}

bool writeDailyDoseAmtToDb(String color, int amt){
  bool retVal = true;
  //setDate();
  String month = "";
  if(mo == 0){
      month = "January";
  }else if(mo == 1){
    month = "February";
  }else if(mo == 2){
    month = "March";
  }else if(mo == 3){
    month = "April";
  }else if(mo == 4){
    month = "May";
  }else if(mo == 5){
    month = "June";
  }else if(mo == 6){
    month = "July";
  }else if(mo == 7){
    month = "August";
  }else if(mo == 8){
    month = "Septempber";
  }else if(mo == 9){
    month = "October";
  }else if(mo == 10){
    month = "November";
  }else if(mo == 11){
    month = "December";
  }
  String parentPath = "/Doser/"+color+"/Data/Year/"+yrStr+"/"+month+"/Day_"+daStr;
      //WebSerial.println(path);
 String childPath = "value";

  
  if(db->databaseReady()){
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }else{
    db->initDb();
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }
  childPath.clear();
  return retVal;
}



/*
////////////////////////////////////////////////////////////////////
//
//  Fuction: readFile
//
//  Input:  SPIFFS,
//          char[]  filename path i.e. "/String.txt"
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
String readFile(fs::FS &fs, const char * path) {
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    //Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  //Serial.println(fileContent);
  file.close();
  return fileContent;
}

////////////////////////////////////////////////////////////////////
//
//  Fuction: writeFile
//
//  Input:  SPIFFS,
//          char[] filename path i.e. "/String.txt"
//          String to store
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Writing file: %s\r\n", path);
  //Serial.print("path is : ");
  //Serial.println(path);
  //fs.remove(path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    //WebSerial.println("?EW"); //TODO change to right code
    return;
  }
  
  if (file.print(message)) {
    //Serial.println("- file written");
    if ((strcmp(path, "/timezone.txt") == 0)) {
      //Serial.println("Timezone changed!!!!!!!!11");
    }

  } else {
    Serial.println("- write failed");
  }
}

*/

bool writeDailyDosesToDb(){
  bool retVal = true;
  Serial.println("writing tooooooooo dbbbbbbbbbbbbbb");
  WebSerial.println("writing tooooooooo dbbbbbbbbbbbbbb");
  //TODO TODO TODO daily amount isn't adding
 // blueDailyDoseAmtStr = readFile(SPIFFS, "/blueDailyDoseAmt.txt");
  fileSystem.openFromFile("/blueDailyDoseAmt.txt", blueDailyDoseAmt);
  /*int bluAmt = 0;
  if(blueDailyDoseAmtStr != ""){
    //Serial.println("ERROR writeDaily");
    bluAmt = blueDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyDoseAmtToDb("Blue", blueDailyDoseAmt);  //Blue has to be cap to match database
  //String reset = String(0);
  //writeFile(SPIFFS, "/blueDailyDoseAmt.txt",reset.c_str());
  //greenDailyDoseAmtStr = readFile(SPIFFS, "/greenDailyDoseAmt.txt");
  fileSystem.openFromFile("/greenDailyDoseAmt.txt", greenDailyDoseAmt);
 /* int grnAmt = 0;
  if(greenDailyDoseAmtStr != ""){
    grnAmt = greenDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyDoseAmtToDb("Green", greenDailyDoseAmt);
  //writeFile(SPIFFS, "/greenDailyDoseAmt.txt",reset.c_str());
  //yellowDailyDoseAmtStr = readFile(SPIFFS, "/yellowDailyDoseAmt.txt");
  fileSystem.openFromFile("/greenDailyDoseAmt.txt", yellowDailyDoseAmt);
  /*int ylwAmt = 0;
  if(yellowDailyDoseAmtStr != ""){
    ylwAmt = yellowDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyDoseAmtToDb("Yellow", yellowDailyDoseAmt);
  //writeFile(SPIFFS, "/yellowDailyDoseAmt.txt",reset.c_str());
  //purpleDailyDoseAmtStr = readFile(SPIFFS, "/purpleDailyDoseAmt.txt");
  fileSystem.openFromFile("/greenDailyDoseAmt.txt", purpleDailyDoseAmt);
  /*int purAmt = 0;
  if(purpleDailyDoseAmtStr != ""){
    purAmt = purpleDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyDoseAmtToDb("Purple", purpleDailyDoseAmt);
 // writeFile(SPIFFS, "/purpleDailyDoseAmt.txt",reset.c_str());
  return retVal;
}

bool writeCalibrationToDb(int color, float amt){
  bool retVal = true;
  String parentPath = "";
  String childPath = "";

  if(color == 1){
    parentPath = "/Doser/Blue/Calibration";
    childPath = "secBluePerMl";
  }else if(color == 2){
    parentPath = "/Doser/Green/Calibration";
    childPath = "secBluePerMl";
  }else if(color == 3){
    parentPath = "/Doser/Yellow/Calibration";
    childPath = "secBluePerMl";
  }else if(color == 4){
    parentPath = "/Doser/Purple/Calibration";
    childPath = "secBluePerMl";
  }


  if(db->databaseReady()){
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }else{
    db->initDb();
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }
  childPath.clear();
  return retVal;
}

void setDate(){
  mySched->update();
  yr = mySched->getYear();
  yr = yr+1900;
  yrStr = String(yr);
  Serial.print("Year is: ");
  Serial.println(yrStr);
  mo = mySched->getMonth();
  Serial.print("Month is: ");
  Serial.println(mo);

  da = mySched->getDay();
  daStr = String(da);
  Serial.print("Day is: ");
  Serial.println(daStr);
 
  fileSystem.saveToFile("/curDay.txt", daStr);

}