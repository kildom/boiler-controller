
#include <vector>

#include "model.hpp"

#include "lowlevel.hh"
#include "log.hh"

#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))


static State state;
static bool startupDone = false;
static double timeoutLeft = (double)PERIODIC_TIMEOUT / 1000.0;
static std::vector<uint8_t> commReceiveBuffer(0);
static std::vector<uint8_t> commSendBuffer(0);
static std::vector<uint8_t> diagReceiveBuffer(0);
static std::vector<uint8_t> diagSendBuffer(0);

void global_init()
{
}

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
    WASM_IMPORT(commSend)
    void commSend(uint8_t* data, int size);
    commSend(commSendBuffer.data(), commSendBuffer.size());
    commSendBuffer.clear();
}


int diag_free()
{
    return 2048 - diagSendBuffer.size();
}

void diag_append(uint8_t data)
{
    diagSendBuffer.push_back(data);
}

void diag_send()
{
    WASM_IMPORT(diagSend)
    void diagSend(uint8_t* data, int size);
    diagSend(diagSendBuffer.data(), diagSendBuffer.size());
    diagSendBuffer.clear();
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

uint32_t analog_input(int index)
{
    double x;
    switch (index) {
        case 0: x = state.Tpiec; break;
        case 1: x = state.Tpowr; break;
        case 2: x = state.Tele; break;
        case 3: x = state.Tpodl1; break;
        case 4: x = state.Twyj1; break;
        case 5: x = state.Tpodl2; break;
        case 6: x = state.Twyj2; break;
        case 7: x = state.Tzas; break;
        case 8: x = -50; break;
        default: x = 20; break; // TODO: ASSERT
    }
    double raw = -0.2655244 * x * x + 139.607 * x + 34124.1915; // KTY81/210 + 1.5K resistor with 16-bit ADC
    //double raw = -0.19755154254 * x * x + 138.755151256 * x + 27791.13537582; // KTY81/210 + 2.21K resistor with 16-bit ADC
    if (raw < 0) return 0;
    if (raw > 65535) return 65535;
    return (uint32_t)(raw + 0.5);
}


void store_read(int slot, uint8_t* buffer, int size)
{
    WASM_IMPORT(storeRead)
    void storeRead(int slot, uint8_t* buffer, int size);
    storeRead(slot, buffer, size);
}

void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size)
{
    WASM_IMPORT(storeWrite)
    uint32_t storeWrite(uint32_t state, int slot, const uint8_t* buffer, int size);
    *state = storeWrite(*state, slot, buffer, size);
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
    if (diagReceiveBuffer.size() > 0) {
        diag_event(diagReceiveBuffer.data(), diagReceiveBuffer.size());
        diagReceiveBuffer.clear();
    }
    for (int i = 0; i < count; i++) {
        if (timeoutLeft <= maxStepTime) {
            if (timeoutLeft > 0.00000001) {
                state.step(timeoutLeft);
            }
            timeoutLeft = (double)PERIODIC_TIMEOUT / 1000.0;
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

WASM_EXPORT(diagRecv)
uint8_t* diagRecv(int size)
{
    auto oldSize = diagReceiveBuffer.size();
    diagReceiveBuffer.resize(oldSize + size);
    return diagReceiveBuffer.data() + oldSize;
}

WASM_EXPORT(button)
void button(int index, int state)
{
    // TODO: maybe later: button_event(index, state ? true : false);
}
