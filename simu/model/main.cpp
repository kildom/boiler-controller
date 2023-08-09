
#include <vector>

#include "model.hpp"

#include "lowlevel.hh"

#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))


WASM_IMPORT(commSend)
void commSend(uint8_t* data, int size);


static State state;
static bool startupDone = false;
static double timeoutLeft = 1000000000.0;
static std::vector<uint8_t> commReceiveBuffer(0);
static std::vector<uint8_t> commSendBuffer(0);


uint32_t get_time()
{
    return (uint32_t)(uint64_t)(state.Time * 1000.0);
}

void timeout(uint32_t t)
{
    int32_t diff = t - get_time();
    timeoutLeft = (double)diff * 0.001;
}

int comm_free()
{
    return 2048 - commSendBuffer.size();
}

void comm_append(uint8_t data)
{
    commSendBuffer.push_back(data);
}

void comm_send()
{
    commSend(commSendBuffer.data(), commSendBuffer.size());
    commSendBuffer.clear();
}

void output(int index, bool value)
{
    switch (index) {
        case 0: state.R0 = value; break;
        case 1: state.R1 = value; break;
        case 2: state.R2 = value; break;
        case 3: state.R3 = value; break;
        case 4: state.R4 = value; break;
        case 5: state.R5 = value; break;
        case 6: state.R6 = value; break;
        case 7: state.R7 = value; break;
        case 8: state.R8 = value; break;
        case 9: state.R9 = value; break;
        case 10: state.R10 = value; break;
        case 11: state.R11 = value; break;
        case 12: state.R12 = value; break;
        case 13: state.R13 = value; break;
    }
}

void store_read(uint8_t* buffer, int size)
{
    WASM_IMPORT(storeRead)
    void storeRead(uint8_t* buffer, int size);
    storeRead(buffer, size);
}

void store_write(const uint8_t* buffer, int size)
{
    WASM_IMPORT(storeWrite)
    void storeWrite(const uint8_t* buffer, int size);
    storeWrite(buffer, size);
}

WASM_EXPORT(steps)
void steps(int count, double maxStepTime)
{
    if (!startupDone) {
        startup_event();
        startupDone = true;
    }
    if (commReceiveBuffer.size() > 0) {
        comm_event(commReceiveBuffer.data(), commReceiveBuffer.size());
        commReceiveBuffer.clear();
    }
    for (int i = 0; i < count; i++) {
        if (timeoutLeft <= maxStepTime) {
            if (timeoutLeft > 0.00000001) {
                state.step(timeoutLeft);
            }
            timeoutLeft = 1000000000.0;
            timeout_event();
        } else {
            timeoutLeft -= maxStepTime;
            state.step(maxStepTime);
        }
    }
}

WASM_EXPORT(getState)
State* getState() {
    return &state;
}

WASM_EXPORT(commRecv)
uint8_t* commRecv(int size)
{
    auto oldSize = commReceiveBuffer.size();
    commReceiveBuffer.resize(oldSize + size);
    return commReceiveBuffer.data() + oldSize;
}

WASM_EXPORT(button)
void button(int index, int state)
{
    button_event(index, state ? true : false);
}
