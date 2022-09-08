#ifndef FBDB___h
#define FBDB___h


#if defined(ESP32)
#include <WiFi.h>
#include "FirebaseESP32.h"
#include <Firebase.h>
#elif defined(ESP8266)
#endif

//Provide the token generation process info.
//#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
//#include <addons/RTDBHelper.h>



//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyCq0NqOvq6DMjdW2wu3P0sEPLV-3PuIcKs"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://esp32-firebase-example-852b0-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "mmkkrood@aol.com"
#define USER_PASSWORD "Skippy"


class Database
{
  
  public:

    Database();
    void initDb();
    bool databaseReady();
    bool putData(String path, FirebaseJson json);
    static void streamCallback(StreamData data);
    static void streamTimeoutCallback(bool timeout);
    String putFloat(String nam, float val);
    String putString(String nam, String val);
    String putInt(String nam, int val);
    String putBool(String nam, bool val);
    float getFloat(String nam);
    String getStr(String nam);
    int getInt(String nam);
    bool getBool(String nam);
    FirebaseJson* getJson(String path);
    
    bool putDailyDoseData(String parent, String child, int amt);
    bool putDailyAfillData(String parent, String child, int amt);
    bool putColorMlSec(String parent, String child, int amt);
    
  private:
    //Define Firebase Data object
    //Firebase *fb;

    
    FirebaseAuth auth;
    FirebaseConfig config;
    //static FirebaseData stream;

};


#endif