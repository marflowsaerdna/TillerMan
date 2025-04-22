#include <Arduino.h>
#include "MyIoPort.h"


    // Konstruktor: Minimal(Ausgang)
    MyIoPort::MyIoPort(uint8_t portPin, uint8_t portMode, uint8_t initState, uint8_t activeState)
    {
        pulseMode = false;
        myPin = portPin;
        myMode = portMode;
        myActiveState = activeState;
        pinMode(myPin,  portMode);
        digitalWrite(myPin, initState);
        durationTimer = xTimerCreate("BleTimer", pdMS_TO_TICKS(1000), pdFALSE, this, durationTimerCallback);  // Timer erzeugen, nicht starten
    }

    // Konstruktor: I/O
    MyIoPort::MyIoPort(uint8_t portPin, uint8_t portMode, uint8_t initState, uint8_t activeState, uint32_t longTime, IoPortCallback onPressCallback, IoPortDurationCallback onReleaseCallback, IoPortCallback onLongPressCallback)
    {
        pulseMode = false;
        myPin = portPin;
        myMode = portMode;
        myActiveState = activeState;
        myLongTime = longTime;
        onPress = onPressCallback;
        onRelease = onReleaseCallback;
        onLongPress = onLongPressCallback;
        pinMode(myPin,  portMode);
        digitalWrite(myPin, initState);
        durationTimer = xTimerCreate("BleTimer", pdMS_TO_TICKS(longTime), pdFALSE, this, durationTimerCallback);  // Timer erzeugen, nicht starten
        debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(BTN_DEBOUNCE), pdFALSE, this, debounceTimerCallback);
        if (portMode != OUTPUT)
            attachInterruptArg(myPin, isrHandler, this, CHANGE); // wenn als Eingang definiert, Interrupt auf beide Flanken setzen
    }



    // Port-Zustand lesen
    bool MyIoPort::read()
    {
        return digitalRead(myPin);
    }

    // Negativen Impuls mit bestimmter Dauer ausgeben
    void MyIoPort::pulse(unsigned long durationMs)
    {
        pulseMode = true;
        detachInterrupt(myPin);         // Flankeninterrupts während des Puls sperren
        pinMode(myPin, OUTPUT);         // Pin ist Output
        digitalWrite(myPin, myActiveState);
        xTimerChangePeriod(durationTimer, durationMs, 0);   // Timer für die Pulsdauer setzten
        xTimerStart(durationTimer, 0);
    }

    // Statische Callback-Funktion
    void MyIoPort::durationTimerCallback(TimerHandle_t xTimer)
    {
        MyIoPort *instance = static_cast<MyIoPort *>(pvTimerGetTimerID(xTimer));
        if (instance) {
            if (instance->pulseMode == true )       // Bei pulsMode aktiv ist es das Ende des Aktivpulses
            {
                // Serial.println("Pulse End");
                digitalWrite(instance->myPin, !instance->myActiveState);            // Pin zurück auf initial setzen
                pinMode(instance->myPin,  instance->myMode);                        // Pin mode zurückstellen
                instance->pulseMode = false;                                        // Pulsmode beenden
                attachInterruptArg(instance->myPin, isrHandler, instance, CHANGE);  // Flanken-Interrupts wieder erlauben
            }
            else                                  // Bei Eingang ist es der longpressed Timer
            {
                // Serial.println("Taste Longpress");
                if (instance->onLongPress)                  // Callback aufrufen, wenn gesetzt
                    instance->onLongPress();
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xTimerStartFromISR(instance->durationTimer, &xHigherPriorityTaskWoken);      // longtime bleibt weiter gesetzt
            }

        }
    }
     
    void IRAM_ATTR MyIoPort::isrHandler(void* arg) {
        MyIoPort* instance = static_cast<MyIoPort*>(arg);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(instance->debounceTimer, &xHigherPriorityTaskWoken);
    }

    void MyIoPort::debounceTimerCallback(TimerHandle_t xTimer) {
        MyIoPort* instance = static_cast<MyIoPort*>(pvTimerGetTimerID(xTimer));
        if (digitalRead(instance->myPin) == instance->myActiveState) {            // Aktivstatus festgestellt
            instance->pressStartTime = millis(); // Zeitstempel setzen
            // Serial.println("Taste gedrueckt");
            if (instance->onPress)                  // Callback aufrufen, wenn gesetzt
                instance->onPress();
            if (instance->myMode != OUTPUT)         // Wenn Input, longpress Timer starten
            {
                xTimerChangePeriod(instance->durationTimer, instance->myLongTime, 0);
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xTimerStartFromISR(instance->durationTimer, 0);
            }
        } else {
            Serial.println("Taste losgelassen");
            uint32_t pressDuration = millis() - instance->pressStartTime;
            if (instance->onRelease)                  // Callback aufrufen, wenn gesetzt
                instance->onRelease(pressDuration);
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTimerStopFromISR(instance->durationTimer, &xHigherPriorityTaskWoken);
        }
    }

    void MyIoPort::testPulse(MyIoPort* instance)
    {
                // Port einmal toggeln
        digitalWrite(instance->myPin, !digitalRead(instance->myPin));
        delay(5);
        digitalWrite(instance->myPin, !digitalRead(instance->myPin));
        pinMode(instance->myPin, instance->myMode);

    }