

#include <stdint.h>
#include <stddef.h>

#include <algorithm>
#include <string>
#include <cstring>

#include "../../src/control/controlInterface.hh"

#include "modelCommon.hh"
#include "model.hh"
#include "../../src/control/log.hh"
#include "modelHandler.hh"

enum CmdType {
    TYPE_DIAG = 1 << 5,
    TYPE_READ = 2 << 5,
    TYPE_WRITE = 3 << 5,
    TYPE_RUN = 4 << 5,
    TYPE_INIT = 5 << 5,
    TYPE_RESET = 6 << 5,
    TYPE_MASK = 7 << 5,
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
            uint8_t data[255 - 2];
        } write;
        struct
        {
            uint8_t data[255];
        } diag;
        struct
        {
            uint16_t offset;
            uint16_t size;
        } readRequest;
        struct
        {
            uint8_t data[255];
        } readResponse;
        struct
        {
            uint8_t data[255];
        } init;
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
static uint8_t* const stateBufferBegin = (uint8_t*)&modelState._MODEL_BEGIN_STATE_FIELD;
static uint8_t* const paramsBufferBegin = (uint8_t*)&modelState._MODEL_BEGIN_PARAMS_FIELD;
static uint8_t* const paramsBufferEnd = (uint8_t*)&modelState._MODEL_END_PARAMS_FIELD + sizeof(modelState._MODEL_END_PARAMS_FIELD);
static const size_t stateSize = paramsBufferEnd - stateBufferBegin;
static Packet packet;
static uint8_t* const packetBuffer = (uint8_t*)&packet;
static size_t packetSize = 0;
static fptype timeoutLeft;

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
    static fptype simulationRunTime = 0.0_f;
    static fptype expectedSimulationRunTime = 0.0_f;

    uint32_t realTimeValue = modelPortTime();

    if (resetTimeState) {
        lastRealTimeValue = realTimeValue;
        expectedSimulationRunTime = simulationRunTime;
    }

    uint32_t realTimeDelta = (realTimeValue - lastRealTimeValue) & ((((1 << (TIME_BITS - 1)) - 1) << 1) + 1);
    lastRealTimeValue = realTimeValue;

    expectedSimulationRunTime += (fptype)realTimeDelta * 0.001_f;

    fptype runTimeToDo = std::min((fptype)maxSimuTimeMs * 0.001_f, expectedSimulationRunTime - simulationRunTime);

    if (runTimeToDo < 0.001_f) {
        return;
    }

    fptype simulationTimeStart = modelState.Time;
    fptype simulationTimeEnd = modelState.Time + runTimeToDo * modelState.speed;
    fptype maxStepTime = (fptype)maxStepTimeMs * 0.001_f;

    modelEnsureStartup();
    int realTimeQueryCounter = 0;

    while (modelState.Time < simulationTimeEnd && modelPortIsEmpty()) {
        if (timeoutLeft <= maxStepTime) {
            if (timeoutLeft > 0.00000001_f) {
                modelState.step(timeoutLeft);
            }
            timeoutLeft = (fptype)PERIODIC_TIMEOUT / 1000.0_f;
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
    size_t offsetBegin = 0;
    size_t offsetEnd = 0;
    uint16_t maxStepTimeMs;
    uint16_t maxSimuTimeMs = 0;
    bool resetTimeState;

    switch (packet.type & TYPE_MASK)
    {
    case TYPE_INIT:
        modelEnsureStartup();
        packet.init.data[packet.dataSize - 1] = sizeof(fptype);
        modelPortAppend(packetBuffer, (size_t)packet.dataSize + 2);
        modelPortSend();
        break;

    case TYPE_DIAG:
        modelEnsureStartup();
        diag_event(packet.diag.data, packet.dataSize);
        packet.dataSize = 0;
        modelPortAppend(packetBuffer, 2);
        modelPortSend();
        break;

    case TYPE_RUN:
        modelEnsureStartup();
        maxStepTimeMs = packet.run.maxStepTimeMs;
        maxSimuTimeMs = packet.run.maxSimuTimeMs;
        resetTimeState = !!packet.run.resetTimeState;
        offsetBegin = 0;
        offsetEnd = paramsBufferBegin - stateBufferBegin;
        __attribute__((fallthrough)); // no break here

    case TYPE_READ:
        if (offsetEnd == 0) {
            offsetBegin = (size_t)packet.readRequest.offset;
            offsetEnd = offsetBegin + (size_t)packet.readRequest.size;
        }
        for (offsetBegin = offsetBegin; offsetBegin <= offsetEnd; offsetBegin += 255) {
            packet.dataSize = std::min((size_t)255, offsetEnd - offsetBegin);
            std::memcpy(packet.readResponse.data, &stateBufferBegin[offsetBegin], packet.dataSize);
            modelPortAppend(packetBuffer, (size_t)packet.dataSize + 2);
        }
        modelPortSend();
        if (maxSimuTimeMs > 0) {
            simulate(maxStepTimeMs, maxSimuTimeMs, resetTimeState);
        }
        break;
  
    case TYPE_WRITE:
        std::memcpy(&stateBufferBegin[packet.write.offset], packet.write.data, packet.dataSize - 2);
        packet.dataSize = 0;
        modelPortAppend(packetBuffer, 2);
        modelPortSend();
        break;
    default:
        ERR("Unknown model packet type.");
    }
}

void modelPortEvent(const uint8_t* data, size_t size)
{
    while (packetSize == 0 && *data == 0 && size > 0) {
        data++;
        size--;
    }
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
    fptype x;
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
    //fptype raw = -0.2655244_f * x * x + 139.607_f * x + 34124.1915_f; // KTY81/210 + 1.5K resistor with 16-bit ADC
    fptype raw = -0.19755154254_f * x * x + 138.755151256_f * x + 27791.13537582_f; // KTY81/210 + 2.21K resistor with 16-bit ADC
    if (raw < 0) return 0;
    if (raw > 65535) return 65535;
    return (uint32_t)(raw + 0.5_f);
}

uint32_t get_time()
{
    return (uint32_t)(uint64_t)(modelState.Time * 1000.0_f);
}

void timeout(uint32_t t)
{
    int32_t diff = t - get_time();
    timeoutLeft = (fptype)diff * 0.001_f;
}

void store_read(int slot, uint8_t* buffer, int size); // Implemented by low-level
void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size); // Implemented by low-level

int comm_free(); // Implemented by low-level
void comm_append(uint8_t data); // Implemented by low-level
void comm_send(); // Implemented by low-level


int diag_free()
{
    int free = modelPortFree() - stateSize - (stateSize + 254) / 255 * 2 - 32;
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
