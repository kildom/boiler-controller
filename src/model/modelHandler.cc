

#include <stdint.h>
#include <stddef.h>

#include <algorithm>

#include "../control/controlInterface.hh"

#include "model.hh"
#include "modelHandler.hh"

enum CmdType {
    CMD_DIAG = 0xEE,
};

State modelState;

uint32_t rxBuffer[(2 + 255 + 3) / 4];
size_t rxBufferLength = 0;
uint32_t txBuffer[(2 + 255 + 3) / 4];
size_t txBufferLength = 0;

uint8_t diagBuffer[2048 + 2];
size_t diagBufferLength = 0;

void modelDataOnReceived(const uint8_t* data, size_t size)
{
    /*
    Handler is responsible for running the simulation at specific speed.
    It will run it in this function.
    It will adjust the speed parameter if to fast.
    When running, it should check if there are data from different sources, e.g. comm UART
    Call modelCommReceive() to get incoming comm data and call comm_event() if any.
    Sender will send real time in incoming data.
    Header (request and response):
        u8 cmd
        u8 packetSize
    Packets:
        Diag:
            u8[] data
            response:
                u8[] data - diag data
        Write:
            u16 offset
            u8[] data
            response:
                empty
        Read:
            u16 offset
            u16 size
            response:
                u8[] data
        Run:
            double maxStepTime [sec]
            double realTime [sec]
            response:
                u8[] data - diag data
    */
}

void global_init(); // Implemented by low-level

void output(int index, bool value)
{
    switch (index) {
        case 0: modelState.R0 = value; break;
        case 1: modelState.R1 = value; break;
        case 2: modelState.R2 = value; break;
        case 3: modelState.R3 = value; break;
        case 4: modelState.R4 = value; break;
        case 5: modelState.R5 = value; break;
        case 6: modelState.R6 = value; break;
        case 7: modelState.R7 = value; break;
        case 8: modelState.R8 = value; break;
        case 9: modelState.R9 = value; break;
        case 10: modelState.R10 = value; break;
        case 11: modelState.R11 = value; break;
        case 12: modelState.R12 = value; break;
        case 13: modelState.R13 = value; break;
    }
}

bool input(int index)
{
    switch (index) {
        case 0:
            return modelState.IN0;
            break;
        case 1:
            return modelState.IN1;
            break;
        default:
            // TODO: fatal error
            return false;
    }
}


uint32_t analog_input(int index)
{
    double x;
    switch (index) {
        case 0: x = modelState.Tpiec; break;
        case 1: x = modelState.Tpowr; break;
        case 2: x = modelState.Tele; break;
        case 3: x = modelState.Tpodl1; break;
        case 4: x = modelState.Twyj1; break;
        case 5: x = modelState.Tpodl2; break;
        case 6: x = modelState.Twyj2; break;
        case 7: x = modelState.Tzas; break;
        case 8: x = -50; break;
        default: x = 20; break; // TODO: ASSERT
    }
    //double raw = -0.2655244 * x * x + 139.607 * x + 34124.1915; // KTY81/210 + 1.5K resistor with 16-bit ADC
    double raw = -0.19755154254 * x * x + 138.755151256 * x + 27791.13537582; // KTY81/210 + 2.21K resistor with 16-bit ADC
    if (raw < 0) return 0;
    if (raw > 65535) return 65535;
    return (uint32_t)(raw + 0.5);
}

uint32_t get_time()
{
    return (uint32_t)(uint64_t)(modelState.Time * 1000.0);
}

void timeout(uint32_t t)
{
    int32_t diff = t - get_time();
    modelState.timeoutLeft = (double)diff * 0.001;
}

void store_read(int slot, uint8_t* buffer, int size); // Implemented by low-level
void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size); // Implemented by low-level

int comm_free(); // Implemented by low-level
void comm_append(uint8_t data); // Implemented by low-level
void comm_send(); // Implemented by low-level


int diag_free()
{
    int free = debugPortFree();
    int freeFullPackets = free / 257;
    int freeRemaining = free % 257 - 2;
    if (freeRemaining < 0) freeRemaining = 0;
    int totalFree = freeFullPackets * 255 + freeRemaining;
    return totalFree - diagBufferLength;
}

void diag_append(uint8_t data)
{
    diagBuffer[diagBufferLength++] = data;
}

void diag_send()
{
    size_t index = 2;
    while (index < diagBufferLength) {
        size_t length = std::min(diagBufferLength - index, (size_t)255);
        diagBuffer[index - 2] = CMD_DIAG;
        diagBuffer[index - 1] = length;
        debugPortAppend(&diagBuffer[index - 2], 2 + length);
    }
    debugPortSend();
    diagBufferLength = 2;
}
