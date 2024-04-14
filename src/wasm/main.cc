
#include <vector>
#include <string>
#include <cstring>

#include "../control/controlInterface.hh"
#include "../model/modelHandler.hh"
//#include "log.hh"

#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))

static std::basic_string<uint8_t> commReceiveBuffer(0);
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

WASM_IMPORT(modelDataSend)
void modelDataSend(const uint8_t* data, size_t size);

WASM_EXPORT(modelDataOnReceived)
void modelDataOnReceived(const uint8_t* data, size_t size);

WASM_EXPORT(commRecv)
uint8_t* commRecv(int size)
{
    auto oldSize = commReceiveBuffer.size();
    commReceiveBuffer.resize(oldSize + size);
    return commReceiveBuffer.data() + oldSize;
}

int debugPortFree()
{
    return 2048 - debugSendBuffer.size();
}

void debugPortAppend(uint8_t data)
{
    debugSendBuffer.push_back(data);
}

void debugPortAppend(const uint8_t* data, size_t size)
{
    size_t startOffset = debugReceiveBuffer.size();
    debugSendBuffer.resize(startOffset + size);
    std::memcpy((uint8_t*)debugSendBuffer.c_str() + startOffset, data, size);
}

void debugPortSend()
{
    WASM_IMPORT(debugSend)
    void debugSend(const uint8_t* data, int size);
    debugSend(debugSendBuffer.c_str(), debugSendBuffer.length());
    debugSendBuffer.clear();
}

WASM_EXPORT(debugRecv)
uint8_t* debugRecv(int size)
{
    auto oldSize = debugReceiveBuffer.size();
    debugReceiveBuffer.resize(oldSize + size);
    return debugReceiveBuffer.data() + oldSize;
}
