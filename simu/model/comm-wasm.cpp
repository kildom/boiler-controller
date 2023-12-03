
#include "lowlevel.hh"

#if SIMULATION_LEVEL == SIMULATION_FULL

#include "wasm.hpp"

static const int COMM_PORT = 0;
static const int AUX_PORT = 1;

static uint8_t commBuffer[1024];
static int commBufferSize = 0;

static uint8_t auxBuffer[1024];
static int auxBufferSize = 0;

WASM_IMPORT(uartSend)
void uartSend(int port, uint8_t* data, int size);

WASM_EXPORT(malloc)
void* mallocWASM(size_t size)
{
    return (void*)new uint8_t[size];
}

WASM_EXPORT(free)
void freeWASM(void* data)
{
    delete[] (uint8_t*)data;
}

void comm_init()
{
}

int comm_free()
{
    return sizeof(commBuffer) - commBufferSize;
}
void comm_append(uint8_t data)
{
    if (commBufferSize < sizeof(commBuffer)) {
        commBuffer[commBufferSize++] = data;
    }
}

void comm_send()
{
    uartSend(COMM_PORT, commBuffer, commBufferSize);
    commBufferSize = 0;
}

void aux_init()
{
}

int aux_free()
{
    return sizeof(auxBuffer) - auxBufferSize;
}

void aux_append(uint8_t data)
{
    if (auxBufferSize < sizeof(auxBuffer)) {
        auxBuffer[auxBufferSize++] = data;
    }
}

void aux_append(uint8_t* data, int size)
{
    while (size > 0 && auxBufferSize < sizeof(auxBuffer)) {
        auxBuffer[auxBufferSize++] = *data++;
        size--;
    }
}

void aux_send()
{
    uartSend(AUX_PORT, auxBuffer, auxBufferSize);
    auxBufferSize = 0;
}

void aux_flush(int freeSize)
{
    aux_send();
}

WASM_EXPORT(uartRecv)
void uartRecv(int port, uint8_t* data, int size)
{
    if (port == AUX_PORT) {
        aux_event(data, size);
    } else if (port == COMM_PORT) {
        comm_event(data, size);
    }
}

#endif // SIMULATION_LEVEL == SIMULATION_FULL
