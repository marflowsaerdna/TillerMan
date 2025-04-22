#include "InputParser.h"

static InputParser* instance = nullptr;

InputParser::InputParser(MyBleClient* bleClient, InputCallbackFunction callback)
{   
    instance = this;
    steuerbordButton = new MyIoPort(STEUERBORD, INPUT_PULLUP, HIGH, LOW, DIRBTN_LONGPRESS, onSbButtonPressedStatic, onSbButtonReleasedStatic, onSbButtonLongPressedStatic);
    backbordButton =   new MyIoPort(BACKBORD, INPUT_PULLUP, HIGH, LOW, DIRBTN_LONGPRESS, onBbButtonPressedStatic, onBbButtonReleasedStatic, onBbButtonLongPressedStatic);
    tackButton =       new MyIoPort(TACK, INPUT_PULLUP, HIGH, LOW, TACKBTN_LONGPRESS, onTackButtonPressedStatic, onTackButtonReleasedStatic, onTackButtonLongPressedStatic);

    TackArmed tackReadiness = DISARMED;
    mInputCallback = callback;

    stdbyWatch = new StdbyWatch(STDBY_BTN, onStdbyStatusChangedStatic);
    stdbyWatch->begin();
    mStdbyStatus = StdbyWatch::UNDEFINED;
    mBleClient = bleClient;

    mArmedTimeoutTimer = xTimerCreate("TackTimer", pdMS_TO_TICKS(TACKARM_TIMEOUT), pdFALSE, this, onTackArmedTimeout);
}

// Einfacher Tastendruck
void InputParser::onSbButtonPressed() {
    if (mTackArmed == ARMED)
        mInputCallback(KEY_TACK_SB);
    else
        mInputCallback(KEY_1GRAD_SB);
}

void InputParser::onBbButtonPressed()
{
    if (mTackArmed == ARMED)
        mInputCallback(KEY_TACK_BB);
    else
        mInputCallback(KEY_1GRAD_BB);
}

void InputParser::onTackButtonPressed()
{

}

// Taste wird länger gedrückt
void InputParser::onSbButtonLongPressed()
{
    mInputCallback(KEY_10GRAD_SB);
}

void InputParser::onBbButtonLongPressed()
{
    mInputCallback(KEY_10GRAD_BB);
}

void InputParser::onTackButtonLongPressed()
{
    if (mStdbyStatus = StdbyWatch::ACTIVE)
    {
        mTackArmed = ARMED;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(mArmedTimeoutTimer, &xHigherPriorityTaskWoken);
    }
}

void InputParser::onSbButtonReleased(uint32_t duration) {
}

void InputParser::onBbButtonReleased(uint32_t duration) {
}

void InputParser::onTackButtonReleased(uint32_t duration) {
}

void InputParser::onStdbyStatusChangedStatic(StdbyWatch::StdbyStatus stdbyStatus)
{
    if (instance)
    {
        instance->onStdbyStatusChanged(stdbyStatus);
    }
}

void InputParser::onStdbyStatusChanged(StdbyWatch::StdbyStatus stdbyStatus)
{
    // Standby Button wurde gedrückt
    if (stdbyStatus == StdbyWatch::STANDBY)
    {
        mStdbyStatus = stdbyStatus;
        mInputCallback(KEY_STDBY_STDBY);
    }
    if (stdbyStatus == StdbyWatch::ACTIVE)
    {
        mStdbyStatus = stdbyStatus;
        mInputCallback(KEY_STDBY_ACTIVE);
        // AWAsoll = AWA;
    }
}

// Definition statische Wrapperfunktionen für Callbacks
void InputParser::onSbButtonPressedStatic() {
    if (instance)
        instance->onSbButtonPressed();
}

void InputParser::onBbButtonPressedStatic()
{
    if (instance)
        instance->onBbButtonPressed();
}

void InputParser::onTackButtonPressedStatic()
{
    Serial.println("Tack Button gedrückt");
    if (instance)
        instance->onTackButtonPressed();
}

// Taste wird länger gedrückt
void InputParser::onSbButtonLongPressedStatic()
{
    if (instance)
        instance->onSbButtonLongPressed();
}

void InputParser::onBbButtonLongPressedStatic()
{
    if (instance)
        instance->onBbButtonLongPressed();
}

void InputParser::onTackButtonLongPressedStatic()
{
    Serial.println("Tack Bereitschaft");
    if (instance)
        instance->onTackButtonLongPressed();
}

void InputParser::onSbButtonReleasedStatic(uint32_t duration) {
    if (instance)
        instance->onSbButtonReleased(duration);
}

void InputParser::onBbButtonReleasedStatic(uint32_t duration) {
    if (instance)
        instance->onBbButtonReleased(duration);
}

void InputParser::onTackButtonReleasedStatic(uint32_t duration) {
    if (instance)
        instance->onTackButtonReleased(duration);
}

void InputParser::onTackArmedTimeout(TimerHandle_t xTimer)
{
    Serial.println("Tack Timeout");
    InputParser* instance = static_cast<InputParser*>(pvTimerGetTimerID(xTimer));
    instance->mTackArmed = DISARMED;
}
