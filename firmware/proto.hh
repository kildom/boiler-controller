#ifndef _PROTO_H_
#define _PROTO_H_

#include "global.hh"

class Proto {
public:

    enum Type {
        LOG = 0x51,
    };

    static void start(Type type);
    static void byte(uint8_t data);
    static void data(const uint8_t* data, size_t size);
    static void end();
    static size_t available();
};

#endif
