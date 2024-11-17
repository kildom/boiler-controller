#ifndef _INPUTS_H_
#define _INPUTS_H_

#include "global.hh"
#include "controlInterface.hh"

class Input {
public:

    enum Index {
        HEAT_ROOM = 0,
        PELLET_PODAWANY = 1,
        POMPA_PELLET = 2,
        INPUTS_COUNT = 3,
    };

    struct Storage {
        uint8_t map[INPUTS_COUNT];
        uint32_t invert; // high level indexing (with mapping)
    };

    static inline bool heatRoom() { return get(HEAT_ROOM); }
    static inline bool pelletPodawany() { return get(PELLET_PODAWANY); }
    static inline bool pompaPellet() { return get(POMPA_PELLET); }

    static bool get(Index index);
};

#endif
