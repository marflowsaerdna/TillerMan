#ifndef MYIOPORT_H
#define MYIOPORT_H

#include <Arduino.h>
#include "hw_def.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class MyIoPort {
public:
    typedef void (*IoPortCallback)();                 // Typ Callback für fallende Flanke und Longpress
    typedef void (*IoPortDurationCallback)(uint32_t); // Typ Callback für steigende Flanke mit Dauer

    // Konstruktor: Pin definieren und als Ausgang setzen
    MyIoPort(uint8_t portPin, uint8_t portMode, uint8_t initState, uint8_t activeState);
    // Konstruktor: Pin definieren und als I/O setzen
    MyIoPort(uint8_t portPin, uint8_t portMode, uint8_t initState, uint8_t activeState, uint32_t longTime, IoPortCallback onPress, IoPortDurationCallback onRelease, IoPortCallback onLongPress);

    // Port-Zustand lesen
    bool read();

    // Negativen Impuls mit bestimmter Dauer ausgeben
    void pulse(unsigned long durationMs);
        void testPulse(MyIoPort* instance);
private:
    void* callerInstance;
    uint8_t myPin, myMode, myActiveState;
    volatile bool pulseMode = false;
    uint32_t myLongTime;
    hw_timer_t *timer = nullptr;
    TimerHandle_t durationTimer, debounceTimer;
    TaskHandle_t  ioTaskHandle;  // Handle für die Task

    static void IRAM_ATTR isrHandler(void* arg);
    static void debounceTimerCallback(TimerHandle_t xTimer);
    static void durationTimerCallback(TimerHandle_t xTimer);

    IoPortCallback onPress, onLongPress;
    IoPortDurationCallback onRelease;

    volatile uint32_t pressStartTime;


    

};
#endif