#ifndef _TEMP_H_
#define _TEMP_H_

#include "global.hh"

class Temp {
public:

    static const int INVALID = -0x80000000;

    enum Index {
        PIEC_PELET = 0,
        PIEC_POWROT = 1,
        PIEC_ELEK = 2,
        PODL1 = 3,
        PODL2 = 4,
        CWU = 5,
        INPUTS_COUNT = 6,
    };

    struct Storage {
        uint8_t map[INPUTS_COUNT];
    };

    // TODO: Maybe cache temperature
    static inline int piecPelet() { return get(PIEC_PELET); }
    static inline int piecPowrot() { return get(PIEC_POWROT); }
    static inline int piecElek() { return get(PIEC_ELEK); }
    static inline int podl1() { return get(PODL1); }
    static inline int podl2() { return get(PODL2); }
    static inline int cwu() { return get(CWU); }
    static inline int podl(int index) { return get(index == 0 ? PODL1 : PODL2); }

    static int get(Index index);

};

#endif
