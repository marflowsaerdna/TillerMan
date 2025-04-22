#ifndef MYOTA_INCLUDED
#define MYOTA_INCLUDED

#include "hw_def.h"


    #ifdef BOOT_WITH_OTA
        #include <WiFi.h>
        #include <WebServer.h>
        #include <ElegantOTA.h>



        void MyOtaSetup();
        void MyOtaLoop();
    #endif
#endif