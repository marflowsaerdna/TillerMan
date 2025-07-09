#ifndef TILLERMAN_H
#define TILLERMAN_H

#include <Arduino.h>
#include "UI/InputParser.h"
#include "BLE/ServerDataFifo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class TillerMan {
public:
        enum ControlStatus : uint8_t {
        READY,
        SERVER_CMD, 
        HOLD_AWA,
        TACK_START_KEYS_SB,
        TACK_START_KEYS_BB,
        TACK_START_CENTRAL,
        TURN_WAIT
    };

    String ControlStatusNames[7] = {
        "READY",
        "SERVER_CMD", 
        "HOLD_AWA",
        "TACK_START_KEYS_SB",
        "TACK_START_KEYS_BB",
        "TACK_START_CENTRAL",
        "TURN_WAIT"
    };

    struct __attribute__((packed)) OpMode {
        bool StdbyActive;
        bool Switch_ON;
    }; 
    
    struct __attribute__((packed)) TillerMgmt {
        char            kennung;
        OpMode          OperationMode;
        short           courseChange;
        short           courseCorr;
        short           AWAsoll;
        short           AWAdelta; 
        short           AWAmove;
        ControlStatus   controlStatus;
    };

    struct CorrectParams {
        InputParser* inputParser;
        int16_t angle;
        uint16_t waitmillis;
    };

    MyIoPort* powerSig;

    TillerMgmt tillerMgmt;
    
    // Konstruktor: Um auf die Ausgänge zu kommen, braucht man den InputParser
    TillerMan(InputParser* inputParser);
    void setServerData(MyBleClient::ServerData serverData);
    void setUserInput(InputParser::UserInput userInput);
    void correctActive(int16_t angle, uint16_t waitmillis);
    static void correctActiveTask(void *pvParameters);
    void mainLoop();

    // Port-Zustand lesen
    bool read();
    InputParser* inputParser;
    // Negativen Impuls mit bestimmter Dauer ausgeben
    void pulse(unsigned long durationMs);
    
private:

    ServerData mServerData;
    bool ServerDataReceived;
    bool ReceivedSomething;
    InputParser::UserInput mUserInput;
    bool userInputFlag = false;

    void* tillerManInstance;
    void sendDataToBLE();
    void manageUserInputStandby(InputParser::UserInput input);
    void manageUserInputActive(InputParser::UserInput input);
    void manageServerData(ServerData data);
    void loopStart();
    void loopEnd();
    short angleAdd(short origin, short adder);
    ControlStatus mainControlStatus = READY;
    

    short AWAalt, AWAdeltaAlt, AWAimpact;
    volatile bool scenarioActive = false;

    
    int led_state;
};
#endif