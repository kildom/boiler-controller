
#include "global.hh"
#include "storage.hh"
#include "relays.hh"

uint32_t Relay::state = 0;

void Relay::set(Index index, bool on)
{
    // TODO: assert(index < OUTPUTS_COUNT)
    int indexLo = storage.relay.map[index];
    uint32_t bitLo = (1 << indexLo);
    if (storage.relay.invert & bitLo) {
        on = !on;
    }
    if (on) {
        state |= bitLo;
    } else {
        state &= ~bitLo;
    }
    output(indexLo, on);
}

bool Relay::get(Index index)
{
    int indexLo = storage.relay.map[index];
    uint32_t bitLo = (1 << indexLo);
    return ((state ^ storage.relay.invert) & bitLo) ? true : false;
}

void Relay::powerOff(Index index)
{
    // TODO: assert(index < OUTPUTS_COUNT)
    int indexLo = storage.relay.map[index];
    output(indexLo, false);
}
