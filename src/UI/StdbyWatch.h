#ifndef STDBYWATCH_H
#define STDBYWATCH_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class StdbyWatch {
  public:

    enum StdbyStatus
    {
        STANDBY,
        ACTIVE, 
        UNDEFINED
    };

    typedef StdbyStatus StdbyStatus_t;
    StdbyStatus stdbyStatus = UNDEFINED;


    typedef void (*StdbyCallback)(StdbyStatus);             // Callback für Status Change

    StdbyWatch(uint8_t pin, StdbyCallback onStdbyChange);
    void begin();

private:
    static void IRAM_ATTR isrHandler(void* arg);
    static void debounceTimerCallback(TimerHandle_t xTimer);
    static void loopTaskStatic(void *pvParameters);
    TaskHandle_t  stdbyWatchTaskHandle;  // Handle für die Task

    uint8_t watchPin;
    static StdbyCallback onStatusChangeStatic;
    StdbyCallback onStdbyChange;
    TimerHandle_t debounceTimer;
};

#endif