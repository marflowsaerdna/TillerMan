#include "ServerDataFifo.h"

ServerDataFifo* ServerDataFifo::instance = nullptr;


ServerDataFifo& ServerDataFifo::getInstance()
{  
    static ServerDataFifo instance;
    return instance;
}

ServerDataFifo::ServerDataFifo(size_t size)
    : capacity(size), head(0), tail(0), count(0)
{

    instance = this; // Setze die statische Instanz
    buffer = new ServerData[capacity];
    return;
}

ServerDataFifo::~ServerDataFifo()
{
    delete[] buffer;
}

bool ServerDataFifo::setAsStruct(const ServerData& data)
{
    bool success = false;
    portENTER_CRITICAL(&mux);
    if (count < capacity) {
        buffer[head] = data;
        head = (head + 1) % capacity;
        count++;
        success = true;
    }
    portEXIT_CRITICAL(&mux);
    // Serial.printf("ServerDataFifo::setAsStruct() called, success: %d, count: %d\n", success, count);
    return success;
}

bool ServerDataFifo::getAsStruct(ServerData& data)
{
    bool success = false;
    portENTER_CRITICAL(&mux);
    if (count > 0) {
        data = buffer[tail];
        tail = (tail + 1) % capacity;
        count--;
        success = true;
    }
    portEXIT_CRITICAL(&mux);
    // Serial.printf("ServerDataFifo::getAsStruct() called, success: %d, count: %d\n", success, count);
    return success;
}

bool ServerDataFifo::set(uint8_t& data)
{
    // Serial.println("ServerDataFifo::set(uint8_t& data) called");

    ServerData sdata;

    memcpy(sdata.asBytes(), &data, ServerData::size());  
    return setAsStruct(sdata); // Verwende die setAsStruct-Methode
}

bool ServerDataFifo::get(uint8_t& data)
{
    ServerData sdata;
    bool success = getAsStruct(sdata); // Verwende die getAsStruct-Methode  
    if (success) {
        memcpy( &data, sdata.asBytes(), ServerData::size());
    }
    return success;
}


bool ServerDataFifo::isEmpty() const
{
    portENTER_CRITICAL(&mux);
    bool result = (count == 0);
    portEXIT_CRITICAL(&mux);
    return result;
}

int ServerDataFifo::getInfo() const
{
    portENTER_CRITICAL(&mux);
    int result = count;
    portEXIT_CRITICAL(&mux);
    return result;
}

bool ServerDataFifo::isFull() const
{
    portENTER_CRITICAL(&mux);
    bool result = (count == capacity);
    portEXIT_CRITICAL(&mux);
    return result;
}

size_t ServerDataFifo::available() const
{
    portENTER_CRITICAL(&mux);
    size_t result = count;
    portEXIT_CRITICAL(&mux);
    return result;
}

void ServerDataFifo::clear()
{
    portENTER_CRITICAL(&mux);
    head = tail = count = 0;
    portEXIT_CRITICAL(&mux);
}