
#include <vector>
#include <string>
#include <cstring>

#include "../control/controlInterface.hh"
#include "../model/modelHandler.hh"

#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))

static std::basic_string<uint8_t> commSendBuffer(0);
static std::basic_string<uint8_t> debugReceiveBuffer(0);
static std::basic_string<uint8_t> debugSendBuffer(0);

void global_init()
{
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

WASM_EXPORT(commRecv)
void commRecv(uint8_t* data, int size)
{
    modelEnsureStartup();
    comm_event(data, size);
}


int modelPortFree()
{
    return 3 * 1024 - debugSendBuffer.size();
}

bool modelPortIsEmpty()
{
    return debugSendBuffer.empty();
}

void modelPortAppend(uint8_t data)
{
    debugSendBuffer.push_back(data);
}

void modelPortAppend(const uint8_t* data, size_t size)
{
    size_t startOffset = debugSendBuffer.size();
    debugSendBuffer.resize(startOffset + size);
    std::memcpy((uint8_t*)debugSendBuffer.c_str() + startOffset, data, size);
}

void modelPortSend()
{
    WASM_IMPORT(modelSend)
    void modelSend(const uint8_t* data, int size);
    modelSend(debugSendBuffer.c_str(), debugSendBuffer.length());
    debugSendBuffer.clear();
}

uint32_t modelPortTime()
{
    WASM_IMPORT(time)
    uint32_t wasmTime(void);
    return wasmTime();
}

void modelPortReset()
{
    WASM_IMPORT(reset)
    void wasmReset(void);
    wasmReset();
}

WASM_EXPORT(modelRecv)
void modelRecv(uint8_t* data, int size)
{
    modelPortEvent(data, size);
}

WASM_EXPORT(malloc)
void *wasmMalloc(size_t size) {
    return malloc(size);
}

WASM_EXPORT(free)
void wasmFree(void* ptr) {
    free(ptr);
}
