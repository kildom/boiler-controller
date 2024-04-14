

#include <stdint.h>
#include <stddef.h>

#include <algorithm>
#include <string>

#include "../control/controlInterface.hh"

#include "model.hh"
#include "modelHandler.hh"

enum CmdType {
    TYPE_DIAG = 0x00,
    TYPE_READ = 0x20,
    TYPE_WRITE = 0x40,
    TYPE_RUN = 0x60,
    TYPE_MASK = 0xE0,
};

struct Packet
{
    uint8_t type;
    uint8_t dataSize;
    union
    {
        struct
        {
            uint16_t offset;
            uint8_t data[253];
        } write;
        struct
        {
            uint8_t data[255];
        } diag;
        struct
        {
            uint8_t data[255];
        } read;
        struct
        {
            uint16_t maxStepTimeMs;
            uint16_t maxSimuTimeMs;
            uint8_t resetTimeState;
        } run;
        struct
        {
            uint8_t data[255];
        } runResponse;
    };
};

State modelState;

static uint8_t diagBuffer[2048 + 2];
static size_t diagBufferLength = 2;
static uint8_t* const stateBuffer = (uint8_t*)&modelState;
static Packet packet;
static uint8_t* const packetBuffer = (uint8_t*)&packet;
static size_t packetSize = 0;
static double timeoutLeft;

void modelEnsureStartup() {
    static bool startupDone = false;
    if (!startupDone) {
        startup_event();
        startupDone = true;
    }
}

void simulate(uint32_t maxStepTimeMs, uint32_t maxSimuTimeMs, bool resetTimeState)
{
    static uint32_t lastRealTimeValue;
    static double simulationRunTime = 0.0;
    static double expectedSimulationRunTime = 0.0;

    uint32_t realTimeValue = modelPortTime();

    if (resetTimeState) {
        lastRealTimeValue = realTimeValue;
        expectedSimulationRunTime = simulationRunTime;
    }

    uint32_t realTimeDelta = (realTimeValue - lastRealTimeValue) & ((((1 << (TIME_BITS - 1)) - 1) << 1) + 1);
    lastRealTimeValue = realTimeValue;

    expectedSimulationRunTime += (double)realTimeDelta * 0.001;

    double runTimeToDo = std::min((double)maxSimuTimeMs * 0.001, expectedSimulationRunTime - simulationRunTime);

    if (runTimeToDo < 0.001) {
        return;
    }

    double simulationTimeStart = modelState.Time;
    double simulationTimeEnd = modelState.Time + runTimeToDo * modelState.speed;
    double maxStepTime = (double)maxStepTimeMs * 0.001;

    modelEnsureStartup();
    int realTimeQueryCounter = 0;

    while (modelState.Time < simulationTimeEnd && modelPortIsEmpty()) {
        if (timeoutLeft <= maxStepTime) {
            if (timeoutLeft > 0.00000001) {
                modelState.step(timeoutLeft);
            }
            timeoutLeft = (double)PERIODIC_TIMEOUT / 1000.0;
            timeout_event();
        } else {
            timeoutLeft -= maxStepTime;
            modelState.step(maxStepTime);
            simulationRunTime += maxStepTime / modelState.speed;
        }
        if (realTimeQueryCounter == 0) {
            realTimeQueryCounter = 20;
            realTimeValue = modelPortTime();
            realTimeDelta = (realTimeValue - lastRealTimeValue) & ((((1 << (TIME_BITS - 1)) - 1) << 1) + 1);
            if (realTimeDelta > maxSimuTimeMs) {
                break;
            }
        }
        realTimeQueryCounter--;
    }
}

void modelPacketReceived()
{
    switch (packet.type & TYPE_MASK)
    {
    case TYPE_DIAG:
        modelEnsureStartup();
        diag_event(packet.diag.data, packet.dataSize);
        packet.dataSize = 0;
        modelPortAppend(packetBuffer, 2);
        modelPortSend();
        break;

    case TYPE_READ:
        for (size_t offset = 0; offset <= sizeof(State); offset += 255) {
            packet.dataSize = std::min((size_t)255, sizeof(State) - offset);
            std::memcpy(packet.read.data, &stateBuffer[offset], packet.dataSize);
            modelPortAppend(packetBuffer, (size_t)packet.dataSize + 2);
        }
        modelPortSend();
        break;
  
    case TYPE_WRITE:
        std::memcpy(&stateBuffer[packet.write.offset], packet.write.data, packet.dataSize - 2);
        packet.dataSize = 0;
        modelPortAppend(packetBuffer, 2);
        modelPortSend();
        break;

    case TYPE_RUN:
        simulate(packet.run.maxStepTimeMs, packet.run.maxSimuTimeMs, !!packet.run.resetTimeState);
        packet.dataSize = sizeof(double);
        std::memcpy(&packet.runResponse.data, &modelState.Time, sizeof(double));
        modelPortAppend(packetBuffer, 2 + sizeof(double));
        modelPortSend();
        break;
    }
}

void modelPortEvent(const uint8_t* data, size_t size)
{
    const uint8_t* end = data + size;
    while (data < end) {
        packetBuffer[packetSize++] = *data++;
        if (packetSize >= 2 && packetSize >= (size_t)packet.dataSize + 2) {
            modelPacketReceived();
            packetSize = 0;
        }
    }
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
    timeoutLeft = (double)diff * 0.001;
}

void store_read(int slot, uint8_t* buffer, int size); // Implemented by low-level
void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size); // Implemented by low-level

int comm_free(); // Implemented by low-level
void comm_append(uint8_t data); // Implemented by low-level
void comm_send(); // Implemented by low-level


int diag_free()
{
    int free = modelPortFree() - sizeof(State) - (sizeof(State) + 254) / 255 * 2 - 32;
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
        diagBuffer[index - 2] = TYPE_DIAG;
        diagBuffer[index - 1] = length;
        modelPortAppend(&diagBuffer[index - 2], 2 + length);
        index += length;
    }
    modelPortSend();
    diagBufferLength = 2;
}
