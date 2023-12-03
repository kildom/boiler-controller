

#include "global.hh"
#include "storage.hh"


void GlobalInit::init()
{
    static bool initialized = false;
    if (!initialized) {
        env_init();
        store_init();
        comm_init();
        diag_init();
        aux_init();
        Storage::init();
        initialized = true;
    }
}

#if SIMULATION_LEVEL == SIMULATION_NONE

void aux_event(uint8_t* data, int size)
{
    diag_event(data, size);
}

#endif
