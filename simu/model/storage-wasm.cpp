#include "lowlevel.hh"

#if SIMULATION_LEVEL == SIMULATION_FULL

#include "wasm.hpp"

void store_init()
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

#endif
