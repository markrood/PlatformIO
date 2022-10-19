#ifndef AWC_H_
#define AWC_H_

#include <Arduino.h>

class Awc{

    public:
        Awc();
        bool doAwc();
        void setFillRunTime(unsigned long amt);
        int getFillRunTime();
        void setEmptyRunTime(unsigned long amt);
        int getEmptyRunTime();




    private:
        //const int HIGHWATER = 13;
        const int NORMALWATER = 16;
        const int LOWWATER = 17;
        //const int ROPUMP = 18;
        const int NEWPUMP = 19;
        const int EMPTYPUMP = 21;

        bool awcRunning = false;
        bool emptyRunning = false;
        bool fillRunning = false;
        bool fillFirstTime = true;
        bool emptyFirstTime = true;

        unsigned long emptyRunTime = 0;
        unsigned long fillRunTime = 0;

        unsigned long elapse = 0;

        const unsigned long MAXFILLTIME = 120000;  //TODO  these are hard coded need to make DB
        const unsigned long MAXEMPTYTIME = 120000;

};
#endif