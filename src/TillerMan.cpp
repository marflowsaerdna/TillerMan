#include "TillerMan.h"
#include "BLE/MyBleClient.h"



TillerMan::TillerMan(InputParser* parser)
{
    powerSig = new MyIoPort(POWERSIG, INPUT_PULLDOWN, 0, 1);
    inputParser = parser;
    tillerManInstance = this;
    memset(&tillerMgmt, 0, sizeof(tillerMgmt));
    tillerMgmt.AWAsoll = 30;
    mServerData.AWA = tillerMgmt.AWAsoll;
}

void TillerMan::manageUserInputStandby(InputParser::UserInput input)
{
    if (input == InputParser::KEY_STDBY_ACTIVE)
    {
        Serial.println("ACTIVE");
        inputParser->mBleClient->sendMessage("ACTIVE");
        tillerMgmt.AWAsoll = mServerData.AWA;
        tillerMgmt.OperationMode.StdbyActive = true;
        tillerMgmt.courseChange = 0;
        mainControlStatus = HOLD_AWA;
    }
}

void TillerMan::manageUserInputActive(InputParser::UserInput input)
{
    switch (input) {
        case InputParser::NO_USER_INPUT: {
            break;
        }
        case InputParser::KEY_1GRAD_SB: {
            Serial.println("1 Grad SB");
            inputParser->mBleClient->sendMessage("1 Grad SB");
            tillerMgmt.AWAsoll = (tillerMgmt.AWAsoll + 1) % 360;
            tillerMgmt.courseChange = 1;
            mainControlStatus = TURN_WAIT;
            break;
        }
        case InputParser::KEY_1GRAD_BB: {
            Serial.println("1 Grad BB");
            inputParser->mBleClient->sendMessage("1 Grad BB");
            tillerMgmt.AWAsoll = (tillerMgmt.AWAsoll + 359) % 360;
            tillerMgmt.courseChange = -1;
            mainControlStatus = TURN_WAIT;
            break;
        }
        case InputParser::KEY_10GRAD_SB: {
            Serial.println("10 Grad SB");
            inputParser->mBleClient->sendMessage("10 Grad SB");
            tillerMgmt.AWAsoll = (tillerMgmt.AWAsoll + 10) % 360;
            mainControlStatus = TURN_WAIT;
            tillerMgmt.courseChange = 10;
            break;
        }
        case InputParser::KEY_10GRAD_BB: {
            Serial.println("10 Grad BB");
            inputParser->mBleClient->sendMessage("10 Grad BB");
            tillerMgmt.AWAsoll = (tillerMgmt.AWAsoll + 350) % 360;
            mainControlStatus = TURN_WAIT;
            tillerMgmt.courseChange = -10;
            break;
        }
        case InputParser::KEY_TACK_SB: {
            Serial.println("Tack SB");
            inputParser->mBleClient->sendMessage("Tack SB");
            mainControlStatus = TACK_START_KEYS_SB;
            break;
        }
        case InputParser::KEY_TACK_BB: {
            Serial.println("Tack BB");
            inputParser->mBleClient->sendMessage("Tack BB");
            mainControlStatus = TACK_START_KEYS_BB;
            break;
        }
        case InputParser::KEY_STDBY_STDBY: {
            // Serial.println("STANDBY");
            inputParser->mBleClient->sendMessage("STANDBY");
            tillerMgmt.courseChange = 0;
            tillerMgmt.OperationMode.StdbyActive = false;
            mainControlStatus = READY;
            break;
        }
        default:
            break;
    }
    
}

void TillerMan::setUserInput(InputParser::UserInput userInput)
{
    mUserInput = userInput;
    userInputFlag = true;
}

void TillerMan::mainLoop()
{   
    ServerDataReceived = ServerDataFifo::getInstance().getAsStruct(mServerData); // Hole die Serverdaten aus der FIFO

    loopStart(); 
    // ServerData serverData;


    if (ServerDataReceived == true)     // nur wenn TillerMan eingeschaltet ist
    {   
        // Serial.println("TillerMan::mainLoop: ServerDataReceived");
        ReceivedSomething = true;
        ServerDataReceived = false;
        if (tillerMgmt.OperationMode.Switch_ON == true)
        {
            if (mServerData.courseChange != 0)          // Server hat einen Kurswechsel angefordert
            {
                inputParser->mBleClient->sendMessage("ServerCMD received");
                mainControlStatus = SERVER_CMD;
            }
            manageServerData(mServerData); // Serverdaten verarbeiten
        }

    }

    if ((userInputFlag == true)) // Tasterfeld im Standby-Modus
    {
        if (tillerMgmt.OperationMode.StdbyActive == true)
        {
            manageUserInputActive(mUserInput);
        }
        else
        {
            // inputParser->mBleClient->sendMessage("User Input, check STANDBY");
            manageUserInputStandby(mUserInput);
        }
        ReceivedSomething = true;
        userInputFlag = false;
    }
    if (ReceivedSomething == true)
    {
        sendDataToBLE();
        ReceivedSomething = false;
    }   
    else
    {
        delay(10);
    }

    loopEnd();
}

void TillerMan::loopStart()
{
    // Serial.println("Loopstart");
    // Schauen, ob TillerMan überhaupt eingeschaltet ist
    tillerMgmt.OperationMode.Switch_ON = powerSig->read();
    // Variablen zum Jonglieren berechnen
    short zwischenErgebnis;
    // Änderung seit dem letzten Durchlauf
    zwischenErgebnis = AWAalt - mServerData.AWA;     
    // if (zwischenErgebnis > 180)
    //    zwischenErgebnis -= 360;
    tillerMgmt.AWAmove   =  zwischenErgebnis; 
    // Serial.printf("AWAmove: %d\n", tillerMgmt.AWAmove);
    // Differenz zwischen Soll und Ist
    zwischenErgebnis = tillerMgmt.AWAsoll - mServerData.AWA;
    if (zwischenErgebnis > 180)
        zwischenErgebnis -= 360;
    tillerMgmt.AWAdelta   =  zwischenErgebnis; 
}

void TillerMan::loopEnd()
{

    // Variablen für den nächsten Durchlauf belegen
    tillerMgmt.courseChange = 0;
    tillerMgmt.courseCorr = 0;
    AWAdeltaAlt = tillerMgmt.AWAdelta;
    AWAalt     = mServerData.AWA;

}

void TillerMan::manageServerData(ServerData serverData)

{
    // Serial.printf("Heap: %d, Min: %d\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());

        //    Serial.print("ServerCMD:  CCR:");
        //    Serial.print(serverData.courseChange);  
        //    Serial.print(":  AWS:");
        //    Serial.println(serverData.AWAsoll); 
    tillerMgmt.controlStatus = mainControlStatus;
    inputParser->mBleClient->sendMessage(ControlStatusNames[mainControlStatus] );
    
    switch (mainControlStatus) 
    {

        case READY:
        {
            // do nothing
            break;
        }
        case SERVER_CMD:
        {
            Serial.print("ServerCMD:  CCR:");
            Serial.print(serverData.courseChange);  
            Serial.print(":  AWS:");
            Serial.println(serverData.AWAsoll); 
            tillerMgmt.AWAsoll = serverData.AWAsoll;
            tillerMgmt.courseChange = serverData.courseChange;
            tillerMgmt.AWAdelta =  AWAalt - serverData.AWAsoll; // Differenz zwischen AWA und AWAsoll
            // inputParser->mBleClient->sendMessage("ServerCMD");
            correctActive(serverData.courseChange, 0);
            if (tillerMgmt.OperationMode.StdbyActive == true) {
                mainControlStatus = TURN_WAIT;
            }
            else {
                mainControlStatus = READY;  // auf neue Anweisungen warten
            }
            break;
        }
        case HOLD_AWA:
        {
            Serial.println("HOLD_AWA");
            if (abs(tillerMgmt.AWAdelta) > 2)
            {
                correctActive(-tillerMgmt.AWAdelta, 0);
                mainControlStatus = TURN_WAIT;
            }
            if (tillerMgmt.AWAdelta == 0)
            {
                tillerMgmt.courseChange = 0;
            }
            break;
        }

        // Für die Entscheidung, ob man sich vor dem Wind oder Am Wind befindet, eignet sich AWA nicht, 
        // deshalb wird (nur) zur Beurteilung des Fahrzustands TWA genommen.
        // Beim Abfallen auf TWA=210/150 kann eigentlich kein AWAsoll berechnet werden. AWAsoll müsste eingelesen werden, wenn sich der Kurs stabiliert hat.
        case TACK_START_KEYS_SB:
        {
            Serial.println("TACK_START_KEYS_SB");
            if ((mServerData.TWA >= 0) && (mServerData.TWA < 90))       // WENDE nach Steuerbord    AWAsoll = 330
            {
                tillerMgmt.AWAsoll = 360 - mServerData.AWA;             // AWA auf die andere Seite spiegeln
                tillerMgmt.courseChange = mServerData.tackAngle;        // Tackjibe_Angle gibt den neuen Kurs vor
                tillerMgmt.courseCorr = tillerMgmt.courseChange - 100;  // Korrektur der festeingestellten 100° Kursänderung
                correctActive((tillerMgmt.courseCorr), 5000);           // TackAngle gibt neuen Kurs an, 100 vom TP abziehen
            }
            if ((mServerData.TWA >= 90)  && (mServerData.TWA < 180))    // ANLUVEN nach Steuerbord  AWAsoll =  30
            {                                                           // Da AWA nicht angefahren werden kann, wird mit TWA <> 45° die Winkeländerung bestimmt
                tillerMgmt.AWAsoll = 30;                                // AWAsoll =  30
                tillerMgmt.courseChange = mServerData.TWA - 45;         // Kursänderung, so dass neuer TWA <> 45
                tillerMgmt.courseCorr = tillerMgmt.courseChange - 100;  // Korrektur der festeingestellten 100° Kursänderung
                correctActive(tillerMgmt.courseCorr, 5000);
            }
            if ((mServerData.TWA >= 180) && (mServerData.TWA < 270))      // HALSE nach Steuerbord    AWAsoll = 150
            {
                tillerMgmt.AWAsoll = 360 - mServerData.AWA;               // AWA auf die andere Seite spiegeln
                tillerMgmt.courseChange = mServerData.tackAngle;          // Tackjibe_Angle gibt den neuen Kurs vor
                tillerMgmt.courseCorr = tillerMgmt.courseChange - 100;    // Korrektur der festeingestellten 100° Kursänderung
                correctActive(tillerMgmt.courseCorr, 5000);               // 60 Grad statt 100
            }
            if ((mServerData.TWA >= 270) && (mServerData.TWA <= 359))     // ABFALLEN nach Steuerbord TWAsoll = 210
            {                                                             // Kursänderung ist von akt. TWA bis 210°, Der Unterschied zwischen TWA und AWAsoll
                tillerMgmt.AWAsoll = 210 + (mServerData.AWA-mServerData.TWA);    // wird auf den Vorwindkurs übernommen. 
                tillerMgmt.courseChange = mServerData.TWA - 210;
                tillerMgmt.courseCorr = tillerMgmt.courseChange - 100;
                correctActive(tillerMgmt.courseCorr, 5000);               // 90 Grad statt 100
            }
            mainControlStatus = TURN_WAIT;
            break;
        }
        case TACK_START_KEYS_BB:
        {
            Serial.println("TACK_START_KEYS_BB");
            if ((mServerData.TWA >= 0) && (mServerData.TWA < 90))         // ABFALLEN nach Backbord  TWAsoll = 150
            {                                                             // Kursänderung ist von akt. TWA bis 150°, Der Unterschied zwischen TWA und AWAsoll
                tillerMgmt.AWAsoll = 150 + (mServerData.AWA-mServerData.TWA);    // wird auf den Vorwindkurs übernommen. 
                tillerMgmt.courseChange = mServerData.TWA - 150;
                tillerMgmt.courseCorr = tillerMgmt.courseChange + 100;
                correctActive(tillerMgmt.courseCorr, 5000);               // 90 Grad statt 100
            }
            if ((mServerData.TWA >= 90)  && (mServerData.TWA < 180))      // HALSE nach Backbord    Werte an 0° spiegeln
            {
                tillerMgmt.AWAsoll = 360 - mServerData.AWA;               // AWA auf die andere Seite spiegeln
                tillerMgmt.courseChange = mServerData.tackAngle - 360;    // Tackjibe_Angle gibt den neuen Kurs vor
                tillerMgmt.courseCorr = tillerMgmt.courseChange + 100;    // Korrektur der festeingestellten 100° Kursänderung
                correctActive(tillerMgmt.courseCorr, 5000);               // 60 Grad statt 100
            }
            if ((mServerData.TWA >= 180) && (mServerData.TWA < 270))      // ANLUVEN nach Backbord  AWAsoll = 330
            {                                                             // Da AWA nicht angefahren werden kann, wird mit TWA <> 315° die Winkeländerung bestimmt
                tillerMgmt.AWAsoll = 330;                                 // AWAsoll = 330
                tillerMgmt.courseChange = mServerData.TWA - 315;          // Kursänderung, so dass neuer TWA <> 315
                tillerMgmt.courseCorr = tillerMgmt.courseChange + 100;    // Korrektur der festeingestellten 100° Kursänderung
                correctActive(tillerMgmt.courseCorr, 5000);
            }
            if ((mServerData.TWA >= 270) && (mServerData.TWA <= 359))     // WENDE nach Backbord    AWAsoll = 30
            {
                tillerMgmt.AWAsoll = 360 - serverData.AWA;                // 30
                tillerMgmt.courseChange = serverData.tackAngle - 360;     // -90
                tillerMgmt.courseCorr = 100 + tillerMgmt.courseChange;    // +10
                correctActive(tillerMgmt.courseCorr, 5000);               // 90 Grad statt 100
            }
            mainControlStatus = TURN_WAIT;

            break;
        }
        case TACK_START_CENTRAL:
        {
            break;
        }
        case TURN_WAIT:
        {
            if (abs(tillerMgmt.AWAdelta) <= 5)                                // Winkel hat sich eingespielt
            {
                Serial.println("Winkel erreicht");
                mainControlStatus = HOLD_AWA;
            }
            if((tillerMgmt.AWAmove == 0) && (abs(tillerMgmt.AWAdelta) > 5))
            {
                correctActive(-tillerMgmt.AWAdelta/2, 0);
            }
            break;
        }
        default:
        {
            Serial.println("Unbekannter Status");
            mainControlStatus = READY;  // auf neue Anweisungen warten
            inputParser->mBleClient->sendMessage("Stat: UNKNOWN->READY"); 
            break;
        }
        tillerMgmt.controlStatus = mainControlStatus; // Status an TillerMgmt übergeben
    }
}

void TillerMan::correctActive(int16_t angle, uint16_t waitmillis)
{
    tillerMgmt.courseChange = angle; // Kursänderung setzen
    // Parameterstruktur dynamisch allozieren
    CorrectParams *params = (CorrectParams *) malloc(sizeof(CorrectParams));
    if (params == NULL) {
        Serial.println("Speicherzuweisung fehlgeschlagen!");
        return;
    }
    
    // Werte setzen
    params->inputParser = inputParser;
    params->angle = angle;
    params->waitmillis = waitmillis;
    
    // Task erstellen
    xTaskCreatePinnedToCore(
        correctActiveTask, // Task-Funktion
        "MyTask",       // Name der Task
        2048,           // Stack-Größe
        params,         // Parameter
        1,              // Priorität
        NULL,           // Task-Handle (nicht benötigt)
        1               // Core 0 (oder 1 für den zweiten Core)
    );
}

void TillerMan::correctActiveTask(void *pvParameters)
{    
    // Parameter aus void* in TaskParams* casten
    CorrectParams* params = (CorrectParams*) pvParameters;

    int16_t zehner = abs(params->angle) / 10;
    int16_t einer = abs(params->angle) % 10;
    MyIoPort* port = nullptr;

    if (params->angle > 0)    // Korrektur nach Steuerbord
    {
        Serial.println("Korr. Steuerb.");
        port = params->inputParser->steuerbordButton;
    }
    else
    {
        Serial.println("Korr. Backb.");
        port = params->inputParser->backbordButton;
    }
    delay(params->waitmillis);
    if (zehner > 0)
    {
        port->pulse(zehner * DIRBTN_LONGPRESS);
        delay((zehner * DIRBTN_LONGPRESS) + 1000);

    }  
    for (int i=0; i < einer; i++)
    {
        port->pulse(DIRBTN_SHORTPRESS);
        delay(DIRBTN_PAUSE + DIRBTN_SHORTPRESS);
    }
    // Speicher freigeben (falls dynamisch alloziert)
    free(params);

    // Task beenden
    vTaskDelete(NULL);

    Serial.println("Korr Ende");
}

void TillerMan::sendDataToBLE()
{
    tillerMgmt.kennung = 'D';  // Kennung für Tiller Management

    /*
    Serial.print(tillerMgmt.kennung);  
    Serial.print(":  Cch:");
    Serial.print(tillerMgmt.courseChange); 
    Serial.print("  Cco:");
    Serial.print(tillerMgmt.courseCorr);
    Serial.print("  AWs:");
    Serial.print(tillerMgmt.AWAsoll);
    Serial.print("  AWd:");
    Serial.print(tillerMgmt.AWAdelta);
    Serial.print("  OpM:");
    Serial.print(tillerMgmt.OperationMode.StdbyActive);
    Serial.println(tillerMgmt.OperationMode.Switch_ON);

    /*
    tillerMgmt.cmd       = 0x0011; //      CMD;
    tillerMgmt.AWAsoll   = 0x2233; // AWAsoll;
    tillerMgmt.AWAdelta  = 0x3344;
    tillerMgmt.AWAmove   = 0x5566;      
*/
    uint8_t buffer[sizeof(tillerMgmt)];   
    memcpy(buffer, &tillerMgmt, sizeof(tillerMgmt));
    inputParser->mBleClient->sendData(buffer, sizeof(buffer));
}

short TillerMan::angleAdd(short origin, short adder)
{
    return (origin + adder + 360) % 360;
}