#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include "hw_def.h"
#include "MyIoPort.h"
#include "StdbyWatch.h"
#include "BLE/MyBleClient.h"



class InputParser {
    public:

        enum TackArmed {
            UNDEIFNED,
            ARMED,
            DISARMED
        };

        StdbyWatch::StdbyStatus mStdbyStatus;
        MyBleClient* mBleClient;



        static void onStdbyStatusChangedStatic(StdbyWatch::StdbyStatus status);
        void onStdbyStatusChanged(StdbyWatch::StdbyStatus status);

        enum UserInput {
            NO_USER_INPUT,
            KEY_1GRAD_SB,
            KEY_1GRAD_BB,
            KEY_10GRAD_SB,
            KEY_10GRAD_BB,
            KEY_TACK_SB,
            KEY_TACK_BB,
            KEY_STDBY_ACTIVE,
            KEY_STDBY_STDBY
        };

        typedef void (*InputCallbackFunction)(UserInput); // Typ Callback für User Befehl
        InputParser(MyBleClient* bleClient, InputCallbackFunction inputCallback);
        
        MyIoPort* steuerbordButton;
        MyIoPort* backbordButton;
        MyIoPort* tackButton;
        
    private:




        UserInput userInput = NO_USER_INPUT;

        TackArmed mTackArmed = TackArmed::UNDEIFNED;
        volatile uint16_t  AWAsoll = 0;

        TimerHandle_t mArmedTimeoutTimer;

        InputCallbackFunction mInputCallback;

        StdbyWatch* stdbyWatch;

        // Static Callbacks
        static void onSbButtonPressedStatic();
        static void onBbButtonPressedStatic();
        static void onTackButtonPressedStatic();

        static void onSbButtonLongPressedStatic();
        static void onBbButtonLongPressedStatic();
        static void onTackButtonLongPressedStatic();

        static void onSbButtonReleasedStatic(uint32_t durance);
        static void onBbButtonReleasedStatic(uint32_t durance);
        static void onTackButtonReleasedStatic(uint32_t durance);

        // Member Callbacks
        void onSbButtonPressed();
        void onBbButtonPressed();
        void onTackButtonPressed();

        void onSbButtonLongPressed();
        void onBbButtonLongPressed();
        void onTackButtonLongPressed();

        void onSbButtonReleased(uint32_t durance);
        void onBbButtonReleased(uint32_t durance);
        void onTackButtonReleased(uint32_t durance);

        static void onTackArmedTimeout(TimerHandle_t xTimer);
  };

#endif