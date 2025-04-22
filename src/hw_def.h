#ifndef HW_DEF_INCLUDED
#define HW_DEF_INCLUDED

#define HW_NANO_ESP32

    // Set LED_BUILTIN if it is not defined by Arduino framework
    #ifdef HW_NANO_ESP32
        #define ACTIVE_LED 13 // 48
        #define STDBY_BTN  2
        #define STEUERBORD 3
        #define BACKBORD   4
        #define TACK       5
        #define POWERSIG   6
    #else
        #define POWER_LED   2
        #define STDBY_BTN   4
        #define STEUERBORD  5
        #define BACKBORD   18
        #define TACK       19
    #endif
    
    // Set LED_BUILTIN if it is not defined by Arduino framework


    // Time definition for Buttons
    #define BTN_DEBOUNCE        10
    #define DIRBTN_SHORTPRESS   50
    #define DIRBTN_LONGPRESS    800
    #define TACKBTN_LONGPRESS   1600
    #define TACKARM_TIMEOUT     5000
    #define DIRBTN_PAUSE        250

    // #define PRINT_WITH_BLE     // uncomment if print and println should be redirected to BLE

    #define BOOT_WITH_OTA      // uncomment if OTA should be used
#endif