//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  sensor 1 is out of water
//
//  Low Water sensor is pin number 17
//  High Water sensor is pin number 13
//  Normal Water sensor is number 16
//
//  RO Pump is number 18
//  New Pump is number 19
//  Empty Pump is number 21
//
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include "ato.h"
#include "awc.h"

#include "AsyncTCP.h"

#include "ESPAsyncWebServer.h"
#include "ESPAsyncWiFiManager.h"         //https://gitWiFiManagerhub.com/tzapu/WiFiManager
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
#include "ArduinoJson.h"
//#include <Wire.h>  // must be included here so that Arduino library object file references work
//#include <RtcDS3231.h>

//RtcDS3231<TwoWire> Rtc(Wire);

AsyncWebServer server(80);
DNSServer dns;
void configModeCallback (AsyncWiFiManager *myWiFiManager);
int addDailyDoseAmt(int color,int val);
bool writeDailyAtoAwcAmtToDb(String atoAwc, int amt);
//void startSpiffs();
//String readFile(fs::FS &fs, const char * path);
//void writeFile(fs::FS &fs, const char * path, const char * message);
bool writeDailyAtoAwcToDb();
bool writeCalibrationToDb(int color, float amt);
void setDate();
int sendHttp(String event);
/*void setTheTime(tm localTime);
bool getNTPtime(int sec);
//bool isDateTimeValid();
//void printDateTime(RtcDateTime now);
void setRtcTime();
int getCurrentTime(String input); */

void doseBlue(int evt);
void doseGreen(int evt);
void doseYellow(int evt);
void dosePurple(int evt);

bool notDosing();

void checkAtoAwcSched(int i);
void doAto(int i);
void doAwc(int i);

int addDailyAtoAwcAmt(int atoAwc,int val);



int led = 2;
FishSched *mySched;
Ato *ato;
Awc *awc;
Database *db;
static Utility *util;
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
int YELLOWMOTOR = 12;
int PURPLEMOTOR = 13;
int LED = 2;

int MINCALTIME = 10;
int count = 0;

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

int atoDailyAmt = 0;
int awcDailyAmt = 0;
String atoDailyAmtStr = "";
String awcDailyAmtStr = "";
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
  bool restart = false;

const char* host = "maker.ifttt.com";  //used in sendHttp
const int httpsPort = 80;  //used in sendHttp
String url = "";  //used in sendHttp
WiFiClient client;  //this is passed into Dosing constructor for connecting to doser
String iPAddress;

int newDay = 0;
//tm timeinfo;
//time_t now;

/*int monthDay;
int currentMonth;
String currentMonthName;
int currentYear;
String formattedTime;
int currentDay;
int currentHour;
int currentMinute;
int currentSecond; */

bool wifiConnected = true; //TODO do this right

int** evtPumpArr;

bool bluePumpPending = false;
bool greenPumpPending = false;
bool yellowPumpPending = false;
bool purplePumpPending = false;
bool blueDosed = false;
bool greenDosed = false;
bool yellowDosed = false;
bool purpleDosed = false;
bool alreadyReset = false;
unsigned long sendDataPrevMillis = 0;

bool midnightDone = false;

bool atoRunning = false;
bool awcRunning = false;

//bool Database::dataChanged = false;

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
  //doser = new Doser(&server,db);
  ato = new Ato();
  awc = new Awc();
  //util = new Utility;
   
  // mySched->initTime();
   mySched->updateMyTime();
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
  //evtPumpArr = db->getEvtPump();
  //util->startSpiffs();
  String test = "test";
  //util->writeFile(SPIFFS,"/test", test.c_str());
  //fileSystem.saveToFile("restart.txt" 0);
  //db->initDb();
  setDate();
   delay(10);
}

void loop() {
//TODO remove just fashing led......
//Serial.println("0");
//mySched->printArray();
  if(blinking){
    digitalWrite(LED, 0);
    blinking = false;
  }else{
    digitalWrite(LED, 1);
    blinking = true;
  }
  //check if any calibraton button is pressed and only do that
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    count++;
    //db->callBack();
  }

  if(Database::dataChanged){
    db->setEvents();
    Database::dataChanged= false;
  }

   while (WiFi.status() != WL_CONNECTED ){
    //TODO db->callBack should be called
    if(w < 2){
      Serial.print('!');
      WebSerial.print("M_wfnc");
      //Serial.println("-1");
      delay(1000);
      w = 0;
      break;
    }else{
      w++;
    }
  }

  
 
    //Serial.println("2");
   // mySched->printArray();
//Serial.println("1.0");
    //Serial.println("in main about to setNowTime");
    mySched->setNowTime(); //initializes time MUST DO THIS
  // mySched->updateMyTime();
  //mySched->printArray();
    mySched->tick(); //sets hour
//mySched->printArray();
    //Serial.print("Hour is: ");
    //Serial.println(nowHour);
//mySched->printArray();
    mySched->tock();//sets minute
//mySched->printArray();
    //Serial.print("Minute is: ");
    //Serial.println(nowMinute);
  
    //std::bitset<25> flags = mySched->getFlags();

    //TODO remove this /////////////////
  // int randNumber = random(31);
  // mySched->resetFlag(randNumber);
  /////////////////////////////////////

//Serial.println("2.0");
//mySched->printArray();
//Serial.println("Just getting for flag for loop");
  int z = 0;
  for(int i= 0;i<32;i++){
      int flagSet = mySched->isFlagSet(i);
      z++;
//Serial.print("z is: ");
//Serial.println(z);
      if(flagSet == 1){
        Serial.println("1");
        if(i != 1){
          if(i !=2){
            WebSerial.print("Event ");
            WebSerial.print(i);
            WebSerial.print(" just fired");
          }else{
            WebSerial.print("-");
          }
        }else{
          WebSerial.print("+");
        }
        //Serial.println("2,1");
        //Serial.print("event in loop is: ");
        //Serial.println(i);
 ////////////////////////////////////////////////////////////////////////////////////////////////////////
  //one_hour=0, fifteen=1, thirty=2, two_hour=3, three_hour=4, 
  //four_hour=5,eight_hour=6, twelve_hour=7,
  //midnight=8, one_am=9, two_am=10, three_am=11, four_am=12, 
  //five_am=13, six_am=14, severn_am=15,
  //eight_amm=16, nine_am=17 ten_am=18, eleven_am=19,
  // noon=20, one_pm=21, two_pm=22, three_pm=23, 
  //four_pm=24, five_pm=25, six_pm=26, severn_pm=27,
  //eight_pmm=28, nine_pm=29 ten_pm=30, eleven_pm=31,
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  checkAtoAwcSched(i);

       }else{
 /*       blueDosed = false;
        greenDosed = false;
        yellowDosed = false;
        purpleDosed = false;
        alreadyReset = false;
        midnightDone = false; */

       } //if 
      //Serial.println("out of flagset loop");
  }//for loop

    //Serial.println("Dam");
    delay(1000);
    //Serial.println(ESP.getFreeHeap(i));
    //WebSerial.println(ESP.getFreeHeap());
    
    //Serial.print(".");
    WebSerial.print(".");
   
}//loop

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

int addDailyAtoAwcAmt(int atoAwc,int val){
  int retVal =0;
//Serial.println("2.2");
  if(atoAwc == 0){
    //Serial.println("2.2.1");
    //blueDailyDoseAmtStr = readFile(SPIFFS, "/blueDailyDoseAmt.txt");
    fileSystem.openFromFile("/atoDailyAmt.txt", atoDailyAmt);
    //Serial.println("2.2.2");
    /*if(blueDailyDoseAmtStr != ""){
      blueDailyDoseAmt = blueDailyDoseAmtStr.toInt();
    }else{
      //TODO
      blueDailyDoseAmt = 0;
    }
    //Serial.println("2.2.3");*/
    atoDailyAmt = atoDailyAmt+val;
    //Serial.println("2.2.4");
    retVal = atoDailyAmt;
    //Serial.println("2.2.5");
    //String blueDailyDoseAmtStrr = String(blueDailyDoseAmt);
    //Serial.println("2.2.6");
    delay(100);
    //util->writeFile(SPIFFS,"/blueDailyDoseAmt.txt", blueDailyDoseAmtStrr.c_str());
    atoDailyAmtStr = String(atoDailyAmt);
    fileSystem.saveToFile("/atoDailyAmt.txt", atoDailyAmtStr.c_str());
    delay(500);
  }else if(atoAwc == 1){
     //Serial.println("2.3.1");
    //greenDailyDoseAmtStr = readFile(SPIFFS, "/greenDailyDoseAmt.txt");
    fileSystem.openFromFile("/awcDailyAmt.txt", awcDailyAmt);
     //Serial.println("2.3.2");
     /*if(greenDailyDoseAmtStr != ""){
      greenDailyDoseAmt = greenDailyDoseAmtStr.toInt();
     }else{
      //TODO
      greenDailyDoseAmt = 0;
     }*/
     //Serial.println("2.3.3");
    awcDailyAmt = awcDailyAmt+val;
     //Serial.println("2.3.4");
    retVal = awcDailyAmt;
     //Serial.println("2.3.5");
    //String geenDailyDoseAmtStrr = String(greenDailyDoseAmt);
       //Serial.println("2.3.6");
  delay(100);
    //util->writeFile(SPIFFS,"/greenDailyDoseAmt.txt", geenDailyDoseAmtStrr.c_str());
    awcDailyAmtStr = String(awcDailyAmt);
    fileSystem.saveToFile("/awcDailyAmt.txt", awcDailyAmtStr.c_str());
     //Serial.println("2.3.7");
     //greenDailyDoseAmtStr.clear();
    // geenDailyDoseAmtStrr.clear();
    delay(500);
  }  
  return retVal;
}

bool writeDailyAtoAwcAmtToDb(String atoAwc, int amt){
  bool retVal = true;
  //setDate();
  String month = "";
  if(mo == 1){
      month = "January";
  }else if(mo == 2){
    month = "February";
  }else if(mo == 3){
    month = "March";
  }else if(mo == 4){
    month = "April";
  }else if(mo == 5){
    month = "May";
  }else if(mo == 6){
    month = "June";
  }else if(mo == 7){
    month = "July";
  }else if(mo == 8){
    month = "August";
  }else if(mo == 9){
    month = "Septempber";
  }else if(mo == 10){
    month = "October";
  }else if(mo == 11){
    month = "November";
  }else if(mo == 12){
    month = "December";
  }
  String parentPath = "/Doser/"+atoAwc+"/Data/Year/"+yrStr+"/"+month+"/Day_"+daStr;
      //WebSerial.println(path);
 String childPath = "value";

  
  if(db->databaseReady()){
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }else{
    db->initDb();
    retVal = db->putDailyDoseData(parentPath, childPath, amt);
  }
  String zero = String(0);
  if(atoAwc == "Ato"){
    fileSystem.saveToFile("/atoDailyAmt.txt", zero.c_str());
  }else if(atoAwc == "Awc"){
    fileSystem.saveToFile("/awcDailyAmt.txt", zero.c_str());
  }
  childPath.clear();
  return retVal;
}




bool writeDailyAtoAwcToDb(){
  bool retVal = true;
  Serial.println("writing tooooooooo dbbbbbbbbbbbbbb");
  WebSerial.println("writing tooooooooo dbbbbbbbbbbbbbb");
  //TODO TODO TODO daily amount isn't adding
 // blueDailyDoseAmtStr = readFile(SPIFFS, "/blueDailyDoseAmt.txt");
  fileSystem.openFromFile("/atoDailyAmt.txt", atoDailyAmt);
  /*int bluAmt = 0;
  if(blueDailyDoseAmtStr != ""){
    //Serial.println("ERROR writeDaly");
    bluAmt = blueDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyAtoAwcAmtToDb("Ato", atoDailyAmt);  //Blue has to be cap to match database
  //String reset = String(0);
  //writeFile(SPIFFS, "/blueDailyDoseAmt.txt",reset.c_str());
  //greenDailyDoseAmtStr = readFile(SPIFFS, "/greenDailyDoseAmt.txt");
  fileSystem.openFromFile("/awcDailyAmt.txt", awcDailyAmt);
 /* int grnAmt = 0;
  if(greenDailyDoseAmtStr != ""){
    grnAmt = greenDailyDoseAmtStr.toInt();
  }*/
  retVal = writeDailyAtoAwcAmtToDb("Awc", greenDailyDoseAmt);
  //writeFile(SPIFFS, "/greenDailyDoseAmt.txt",reset.c_str());
  //yellowDailyDoseAmtStr = readFile(SPIFFS, "/yellowDailyDoseAmt.txt");
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
  mySched->syncTime();
  yr = mySched->getCurrentYear();
  yr = yr;
  yrStr = String(yr);
  Serial.print("Year is: ");
  Serial.println(yrStr);
  mo = mySched->getCurrentMonth();
  Serial.print("Month is: ");
  Serial.println(mo);

  da = mySched->getCurrentDay();
  daStr = String(da-1);  //i need the day for save to db be the day before since i save at midnight
  Serial.print("YesterDay is: ");
  Serial.println(daStr);
 
  fileSystem.saveToFile("/curDay.txt", daStr);

}

//////////////////////////////////////////////////////////////
//                                                          //
//   sentHttp                                               //
//                                                          //
//   input: String message                                  //
//                                                          //
//   output: int (not used)                                 //
//                                                          //
//   description:  this uses the ifttt service to send      //
//                  push notifications                      //
//
//                                                          //
//////////////////////////////////////////////////////////////
int sendHttp(String event) {
  int ret = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print("*");
  }

  //Serial.println("");
  // Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());


  //Serial.print("connecting to ");
  //Serial.println(host + url);
  //  client.setInsecure();
  if (!client.connect(host, httpsPort)) {

    Serial.println("connection failed");
    return 0;
  }
  //Serial.print("requesting URL: ");
  String iftt = "fBplW8jJqqotTqTxck4oTdK_oHTJKAawKfja-WlcgW-";//atoAwcUtil->readFile(SPIFFS, "/ifttt.txt");

  if (event == "Email") {
    //Serial.println("sending email");
    //url = "/trigger/" + event + "/with/key/"+iftt;//+",params={ \"value1\" : \""+iPAddress+"\", \"value2\" : \"02\", \"value3\" : \"03\" }";
    //Serial.println(url);
    //https://maker.ifttt.com/trigger/garage_deur/with/key/d56e34gf756/?value1=8&value2=5&value3=good%20morning
    //TESTING JSON CREATION
    String url = "/trigger/" + event + "/with/key/" + iftt;
    //Serial.println("Starting JSON");
    StaticJsonDocument<80> jdata;
    //        StaticJsonBuffer<69> jsonBuffer;
    String json = "{ \"value1\" : \"atoawc ip: " + iPAddress + "\", \"value2\" : \", atoawc hotspot pw: ato_awc_\", \"value3\" : \", doser hotspot pw : yourdoser\" }";
    auto error = deserializeJson(jdata, json);
    //        JsonObject& root = jsonBuffer.parseObject(json);
    //Serial.println("TESTING POST");

    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 //"Connection: close\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" + // This is the extra CR+LF pair to signify the start of a body
                 json + "\n");
  } else {

    //url = "/trigger/"+event+"/with/key/bOZMgCFy7Bnhee7ZRzyi19";
    url = "/trigger/" + event + "/with/key/" + iftt;

    //Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    //Serial.println("request Sent");
  }
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      // Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');

  //Serial.println("reply was:");
  //Serial.println("==========");
  //Serial.println(line);
  //Serial.println("==========");
  //Serial.println("closing connection");
  ret = 1;
  return ret;
}





void checkAtoAwcSched(int i){
  if(db->isThisEventAtoAwcSet(i,0)){
    doAto(i);
  }
  
  if(db->isThisEventAtoAwcSet(i, 1)){
      doAwc(i);
  }
  if(i == 8 && !midnightDone){
    midnightDone = true;
    setDate();
    mySched->setFlag(i,0);
    mySched->updateMyTime();
    writeDailyAtoAwcToDb();
    WebSerial.print("saved to db");
  }
}


void doAto(int i){
  if(i == 1){
      Serial.println("Fifteen Minutes");
      atoRunning = ato->doAto();
      if(atoRunning && !awcRunning){
      Serial.println("T");
      }else{
        Serial.println("t");
        int amt = ato->getTopOffRunTime();
        int tAmt = addDailyAtoAwcAmt(0,amt);
        mySched->resetFlag(1);
      }
  }
}
void doAwc(int i){
  if(i == 1){
      Serial.println("Fifteen Minutes");
      atoRunning = ato->doAto();
      if(awcRunning && !atoRunning){
      Serial.println("T");
      }else{
        Serial.println("t");
        int amt = ato->getTopOffRunTime();
        int tAmt = addDailyAtoAwcAmt(1,amt);
        mySched->resetFlag(1);
      }
  }
}
