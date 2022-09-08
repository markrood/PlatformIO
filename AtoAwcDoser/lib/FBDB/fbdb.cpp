#include "fbdb.h"

  FirebaseData stream;
  FirebaseData fbdo;
  
Database::Database(){
  
}

void  Database::streamCallback(StreamData data)
{
  Serial.println("in callback");

  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
    //printResult(data); //see addons/RTDBHelper.h
    Serial.print("Stream Path: ");
    Serial.println(data.streamPath());
    Serial.print("Data Path: ");
    Serial.println(data.dataPath());
    Serial.print("Data Type: ");
    Serial.println(data.dataType());
    Serial.print("Event Type: ");
    Serial.println(data.eventType());
    //Serial.print("Data");
    //Serial.println(data.value());
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
      Serial.println(data.to<int>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_float)
      Serial.println(data.to<float>(), 5);
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_double)
      printf("%.9lf\n", data.to<double>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean)
      Serial.println(data.to<bool>()? "true" : "false");
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string)
      Serial.println(data.to<String>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
  {
      FirebaseJson *json = data.to<FirebaseJson *>();
      Serial.println(json->raw());
  }
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_array)
  {
      FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
      Serial.println(arr->raw());
  }


  Serial.println();

  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void Database::streamTimeoutCallback(bool timeout)
{
  //FirebaseData stream;
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void Database::initDb(){
  //FirebaseData stream;
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  //config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

//Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.beginStream(stream, "/test/stream/data"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, this->streamCallback, this->streamTimeoutCallback);

  /** Timeout options, below is default config.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;

  //ESP32 SSL handshake in ms (1 sec - 2 min). This option doesn't allow in ESP8266 core library.
  config.timeout.sslHandshake = 2 * 60 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  */
}
  bool Database::databaseReady(){
    if (Firebase.ready()){
      return true;
    }else{
      return false;
    }
  }

bool Database::putData(String path, FirebaseJson json){
  bool retVal = false;
  retVal = Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json);
    return retVal;
}

  FirebaseJson* Database::getJson(String path){
    bool retVal = false;
    retVal = Firebase.getJSON(fbdo, path);
      FirebaseJson *json = fbdo.jsonObjectPtr();

      //FirebaseJson *jsn = json.to<FirebaseJson *>();
      //Serial.println(json->raw());
      return json;
  }
  
    String Database::putFloat(String nam, float val){
      String retVal = "ok";
    // set value
      if (databaseReady()){
        Firebase.setFloat(fbdo, nam, val);
      // handle error
      }else{        
        String retVal = fbdo.errorReason().c_str();
      }
      return retVal;
    }

    String Database::putString(String nam, String val){
      String retVal = "ok";
    // set value
      if (databaseReady()){
        Firebase.setString(fbdo, nam, val);
      // handle error
      }else{        
        String retVal = fbdo.errorReason().c_str();
      }
      return retVal;
    }
    
    String Database::putInt(String nam, int val){
      String retVal = "ok";
      if (databaseReady() ){
        Firebase.setInt(fbdo, nam, val);
      // handle error
      }else{        
         String retVal = fbdo.errorReason().c_str();
      }
      return retVal;
    }
    
    String Database::putBool(String nam, bool val){
      String retVal = "ok";
      if (databaseReady()){
        Firebase.setBool(fbdo, nam, val);
      // handle error
      }else{        
        String retVal = fbdo.errorReason().c_str();
      }
      return retVal;
    }
    
    float Database::getFloat(String nam){
      float retVal = 0;
      if (databaseReady()){
        retVal = Firebase.getFloat(fbdo, nam);
        float f = fbdo.to<float>();
        retVal = f;
      // handle error
      }else{        
        Serial.println(fbdo.errorReason().c_str());
        retVal = 99;
      }
      return retVal;
    }
    
    String Database::getStr(String nam){
      String retVal = "";
      if (databaseReady()){
        retVal = Firebase.getString(fbdo, nam);
        String s = fbdo.to<String>();
        retVal = s;
      // handle error
      }else{        
        retVal = fbdo.errorReason().c_str();
      }
       return retVal;
    }
    
    int Database::getInt(String nam){
      int retVal;
      if (databaseReady()){
        //Serial.printf("Get int ref... %s\n", Firebase.getInt(fbdo, F(nam.c_str), &retVal) ? String(retVal).c_str() : fbdo.errorReason().c_str());
        retVal = Firebase.getInt(fbdo, nam);
        int i = fbdo.to<int>();
        retVal = i;
      }else{        
        Serial.println(fbdo.errorReason().c_str());
        retVal = 99;
      }
      return retVal;
    }
    
    bool Database::getBool(String nam){
      bool retVal = false;
      // set value
      if (databaseReady()){
        retVal = Firebase.getBool(fbdo, nam);
        bool b = fbdo.to<bool>();
        retVal = b;
      }else{        
        Serial.println(fbdo.errorReason().c_str());
        retVal = false;
      }
      return retVal;
    }

    bool Database::putDailyDoseData(String parent, String child, int amt){
      bool retVal = false;
      FirebaseJson json;
      json.set(child.c_str(), String(amt));
      //retVal = Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json);
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      return retVal;
    }

    bool Database::putColorMlSec(String parent, String child, int amt){
        bool retVal = false;
      FirebaseJson json;
      json.set(child.c_str(), String(amt));
      //retVal = Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json);
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      return retVal;
   
    }

    bool Database::putDailyAfillData(String parent, String child, int amt){
      bool retVal = false;
      FirebaseJson json;
      json.set(child.c_str(), String(amt));
      //retVal = Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json);
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parent.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      return retVal;

    }