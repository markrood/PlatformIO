#ifndef ATO_H_
#define ATO_H_

#include "fishScheduler.h"

//class Ato: public FishSched{
class Ato{
    public:
        Ato();
        bool doAto();


        void setTopOffRunTime(unsigned long val);
        int getTopOffRunTime();


    private:
        bool atoRunning = false;
        bool atoFirstTime = true;
        //const int HIGHWATER = 13;
        const int NORMALWATER = 16;
        //const int LOWWATER = 17;
        const int ROPUMP = 18;
        //const int NEWPUMP = 19;
        //const int EMPTYPUMP = 21;

        const unsigned long MAXTOPOFFTIME = 1200000;  //TODO put this in database right now 2 minutes
        unsigned long elapse = 0;

        unsigned long topOffRunTime = 0;


};
#endif