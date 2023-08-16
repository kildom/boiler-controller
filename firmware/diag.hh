#ifndef DIAG_HH_
#define DIAG_HH_


#include "global.hh"

class Diag {
public:
    enum Mode {
        LOG,
        MENU,
        COMM,
    };

    static void log(const char* text);
    static void update();
    static void write(const char* data, int size, Mode requestMode);
    static void write(uint8_t data, Mode requestMode);
};

#endif /* DIAG_HH_ */
