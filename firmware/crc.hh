#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

class Crc32 {
public:
    uint32_t value;
    Crc32() : value(0) {}
    void reset() { value = 0; }
    uint32_t data(const uint8_t *ptr, uint32_t size);
    uint32_t byte(uint8_t byte);
};

#endif
