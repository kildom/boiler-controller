#ifndef _INPUTS_H_
#define _INPUTS_H_

#include "global.hh"
#include "lowlevel.hh"

class Input {
public:

    enum Index {
        HEAT_ROOM = 0,
        INPUTS_COUNT = 1,
    };

    static inline int heatRoom() { return get(HEAT_ROOM); }

    static int get(Index index) { return input(0); }

};

#endif
