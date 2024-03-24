
#include "lowlevel.hh"

#if SIMULATION_LEVEL > SIMULATION_NONE

#include "model.hpp"

#include "log.hh"

static State state;
static double timeoutLeft = (double)PERIODIC_TIMEOUT / 1000.0;

void env_init()
{
    aux_init();
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

bool input(int index)
{
    switch (index) {
        case 0:
            return state.IN0;
            break;
        case 1:
            return state.IN1;
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
    //double raw = -0.2655244 * x * x + 139.607 * x + 34124.1915; // KTY81/210 + 1.5K resistor with 16-bit ADC
    double raw = -0.19755154254 * x * x + 138.755151256 * x + 27791.13537582; // KTY81/210 + 2.21K resistor with 16-bit ADC
    if (raw < 0) return 0;
    if (raw > 65535) return 65535;
    return (uint32_t)(raw + 0.5);
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

void steps(int count, double maxStepTime)
{
    static bool startupDone = false;

    if (!startupDone) {
        startup_event();
        startupDone = true;
    }

    for (int i = 0; i < count; i++) {
        if (timeoutLeft <= maxStepTime) {
            if (timeoutLeft > 0.000001) {
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

enum PacketType {
    EXECUTE = 1,
    SET_STATE = 3,
    DIAG_RECV = 4,
    DIAG_SEND = 5,
    RESPONSE = 0x40,
};

static const uint32_t PACKET_HEADER_MAGIC = 0xF6A8885A;
static const uint32_t PACKET_HEADER_SIZE = 8;

union Packet {
    struct {
        // Common
        uint32_t magic;
        uint8_t type;
        uint8_t size;
    };
    struct {
        // Execute
        uint32_t _rsv1;
        uint16_t _rsv2;
        uint16_t count;
        uint32_t stateCrc;
    };
    struct {
        uint32_t _rsv3;
        uint16_t _rsv4;
        uint16_t offset;
    };
    struct {
        uint8_t data[255];
    };
    uint8_t raw[256];
};

static Packet packet;
static int receivedPacketBytes = 0;

void preparePacket(int type, int size)
{
    int freeSize = aux_free();
    if (freeSize < sizeof(packet) + size) {
        aux_flush(sizeof(packet) + size);
    }
    memset(&packet, 0, sizeof(packet));
    packet.magic = PACKET_HEADER_MAGIC;
    packet.type = type;
    packet.size = size;
}

void sendPacket() {
    aux_append((uint8_t*)&packet, PACKET_HEADER_SIZE + packet.size);
}

void sendPacket(int type) {
    preparePacket(type, 0);
    sendPacket();
}

static void getState()
{
    uint32_t offset = packet.offset;
    preparePacket(GET_STATE | RESPONSE, 16);
    uint8_t* bufferStart = (uint8_t*)(&state.stateBegin + 1);
    int bufferSize = (uint8_t*)(&state.stateEnd) - bufferStart;
    if (offset > bufferSize) {
        return;
    }
    bufferStart += offset;
    bufferSize -= offset;
    if (bufferSize > 255) {
        bufferSize = 255;
    }
    int freeSize = aux_free();
    if (bufferSize > freeSize - PACKET_HEADER_SIZE) {
        bufferSize = freeSize - PACKET_HEADER_SIZE;
    }
    packet.size = bufferSize;
    memcpy(packet.data, bufferStart, bufferSize);
    sendPacket();
}

static void setState()
{
    uint32_t offset = packet.offset;
    uint32_t size = packet.size;
    uint8_t* bufferStart = (uint8_t*)(&state.stateBegin + 1);
    int bufferSize = (uint8_t*)(&state.stateEnd) - bufferStart;
    if (offset > bufferSize || offset + size > bufferSize) {
        return;
    }
    memcpy(bufferStart + offset, packet.data, size);
    sendPacket(SET_STATE | RESPONSE);
}

static void packetReceived()
{
    switch (packet.type) {
    case EXECUTE:
        steps(packet.count, packet.maxStepTime);
        sendPacket(EXECUTE | RESPONSE);
        break;
    case GET_STATE:
        getState();
        break;
    case SET_STATE:
        setState();
        break;
    case DIAG_RECV:
        diag_event(packet.data, packet.size);
        sendPacket(DIAG_RECV | RESPONSE);
        break;
    }
}

void aux_event(uint8_t* data, int size)
{
    uint8_t* end = data + size;
    for (; data < end; data++) {
        packet.raw[receivedPacketBytes++] = *data;
        if (receivedPacketBytes == 4 && packet.magic != PACKET_HEADER_MAGIC) {
            packet.magic >>= 8;
            receivedPacketBytes--;
        } else if (receivedPacketBytes >= PACKET_HEADER_SIZE && receivedPacketBytes == packet.size + PACKET_HEADER_SIZE) {
            receivedPacketBytes = 0;
            packetReceived();
        }
    }
    aux_send();
}

static uint8_t diagBuffer[128];
static int diagBufferSize = 0;

void diag_init()
{
}

int diag_free()
{
    return sizeof(diagBuffer) - diagBufferSize;
}

void diag_append(uint8_t data)
{
    diagBuffer[diagBufferSize++] = data;
}

void diag_send()
{
    preparePacket(DIAG_SEND, diagBufferSize);
    memcpy(packet.data, diagBuffer, diagBufferSize);
    diagBufferSize = 0;
    sendPacket();
}

#endif // #if SIMULATION_LEVEL > SIMULATION_NONE
