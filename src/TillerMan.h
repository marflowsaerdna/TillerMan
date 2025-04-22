#ifndef TILLERMAN_H
#define TILLERMAN_H

#include <Arduino.h>
#include "UI/InputParser.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class TillerMan {
public:
        enum ControlStatus {
        STANDBY,
        SERVER_CMD, 
        HOLD_AWA,
        TACK_START_KEYS_SB,
        TACK_START_KEYS_BB,
        TACK_START_CENTRAL,

        TURN_WAIT
    };

    struct __attribute__((packed)) TillerMgmt {
        char  kennung;
        bool Stdby;
        short courseChange;
        short courseCorr;
        short AWAsoll;
        short AWAdelta; 
        short AWAmove;
    };

    struct CorrectParams {
        InputParser* inputParser;
        int16_t angle;
        uint16_t waitmillis;
    };

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

    MyBleClient::ServerData mServerData;
    bool serverDataFlag = false;
    InputParser::UserInput mUserInput;
    bool userInputFlag = false;

    void* tillerManInstance;
    void sendDataToBLE();
    void manageUserInputStandby(InputParser::UserInput input);
    void manageUserInputActive(InputParser::UserInput input);
    void manageServerData(MyBleClient::ServerData data);
    void manageServerCmd(MyBleClient::ServerData serverData);
    void loopStart();
    void loopEnd();
    short angleAdd(short origin, short adder);
    ControlStatus mainControlStatus = STANDBY;
    

    volatile short AWAalt, AWAdeltaAlt, AWAimpact;
    volatile bool scenarioActive = false;

    
    int led_state;
};
#endif