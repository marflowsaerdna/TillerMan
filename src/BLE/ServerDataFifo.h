#ifndef SERVERDATAFIFO_H
#define SERVERDATAFIFO_H

#include <Arduino.h>
#include <atomic>

#pragma once

struct __attribute__((packed)) ServerData {
    short MsgCounter;
    short courseChange;
    short AWAsoll;
    short AWA;
    short TWA;
    short tackAngle;

    // Zugriff auf Rohdaten als Bytefolge (read-only)
    const uint8_t* asBytes() const {
        return reinterpret_cast<const uint8_t*>(this);
    }

    // Schreibbarer Zugriff auf Rohdaten (z. B. bei Empfang)
    uint8_t* asBytes() {
        return reinterpret_cast<uint8_t*>(this);
    }

    // Größe in Bytes
    static constexpr size_t size() {
        return sizeof(ServerData);
    }
};

class ServerDataFifo {
public:
    static ServerDataFifo& getInstance();  // Statische Methode, um die Instanz zu erhalten
    ServerDataFifo(size_t size = 10);
    ~ServerDataFifo();

    bool set(uint8_t& data);  // Einfügen
    bool get(uint8_t& data);        // Entnehmen
    bool setAsStruct(const ServerData& data);  // Einfügen
    bool getAsStruct(ServerData& data);        // Entnehmen
    bool isEmpty() const;
    bool isFull() const;
    int getInfo() const;  // Gibt die Anzahl der Elemente in der FIFO zurück
    size_t available() const;
    void clear();

private:
    ServerData* buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;

    static ServerDataFifo* instance;  // Statische Instanzvariable

    // FreeRTOS Spinlock (für ESP32)
    mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};

#endif