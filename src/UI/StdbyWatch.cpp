#include "StdbyWatch.h"

StdbyWatch::StdbyWatch(uint8_t pin, StdbyCallback onChange) {

    watchPin = pin;
    onStdbyChange = onChange;
    debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(800), pdFALSE, this, debounceTimerCallback);
    stdbyStatus = UNDEFINED;

    // **Task erstellen, aber direkt suspendieren**
    xTaskCreate(StdbyWatch::loopTaskStatic, "WatchLoop", 4096, this, 1, &stdbyWatchTaskHandle);
    vTaskSuspend(stdbyWatchTaskHandle);
}

void StdbyWatch::begin() {
    pinMode(watchPin, INPUT_PULLUP);                        // Taster ist normalerweise HIGH
    attachInterruptArg(watchPin, isrHandler, this, CHANGE); // Interrupt auf beide Flanken setzen
    if (digitalRead(watchPin) == HIGH)                      // Dann direkt Timer starten, weil Zustand schon aktiv sein kann
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
    }

}

void StdbyWatch::isrHandler(void* arg) {
    StdbyWatch* instance = static_cast<StdbyWatch*>(arg);
    if (digitalRead(instance->watchPin) == LOW) {
        if (instance->stdbyStatus != STANDBY)         // Status geändert ?
        {
            instance->stdbyStatus = STANDBY;
            if (instance->onStdbyChange)
                vTaskResume(instance->stdbyWatchTaskHandle);    
        }
    }
    else
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(instance->debounceTimer, &xHigherPriorityTaskWoken);
    }
}

void StdbyWatch::debounceTimerCallback(TimerHandle_t xTimer) {
    StdbyWatch* instance = static_cast<StdbyWatch*>(pvTimerGetTimerID(xTimer));
    if (digitalRead(instance->watchPin) == HIGH) {
        instance->stdbyStatus = ACTIVE;
        vTaskResume(instance->stdbyWatchTaskHandle);
    }
}

// **Task-Funktion (darf nicht blockieren)**
void  StdbyWatch::loopTaskStatic(void *pvParameters) {
    StdbyWatch *instance = static_cast< StdbyWatch *>(pvParameters);
    while(1)
    {
        if (instance)
        {
            if (instance->onStdbyChange)
            {
                instance->onStdbyChange(instance->stdbyStatus);    // Callback in upper instance
            }

        }
        vTaskSuspend(instance->stdbyWatchTaskHandle);  // Task wieder pausieren
    }
}