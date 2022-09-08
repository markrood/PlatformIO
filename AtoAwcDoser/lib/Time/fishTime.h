#ifndef FISHTIME_H_
#define FISHTIME_H_

#include <Arduino.h>





class FishTime{
    private:
        //const char* ntpServer = "pool.ntp.org";
        //const char* location  = "America/Chicago";
        const char* ntpServer = "pool.ntp.org";
        const long  gmtOffset_sec = -21600;
            const int   daylightOffset_sec = 3600;
        int currentHour = 99;
        struct tm timeinfo;


    public:
        FishTime();
        void syncTime();
        void initTime();
        bool update();
        int getHour();
        int getMinute();
        int getYear();
        int getMonth();
        int getDay();
};

#endif