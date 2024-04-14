
#include "global.hh"
#include "controlInterface.hh"
#include "proto.hh"
#include "crc.hh"

/* Encoding:
 * Q-Z - header (type)
 * a-p - data
 * A-P - CRC-32
 * 
 * +------+------+ ... +-----+---+---+---+---+
 * | head |  data  ...       |    CRC-32     |
 * +------+------+ ... +-----+---+---+---+---+
 */

#define HEADER_SIZE 1
#define CRC_SIZE (2 * sizeof(crc))

static const uint8_t data_table[] = "abcdefghijklmnop";
static const uint8_t crc_table[] = "ABCDEFGHIJKLMNOP";

static Crc32 crc;

static void data_with_table(const uint8_t* data, uint32_t size, const uint8_t* table)
{
    const uint8_t* end = data + size;
    crc.data(data, size);
    while (data < end) {
        comm_append(table[(*data >> 4) & 0x0F]);
        comm_append(table[*data & 0x0F]);
        data++;
    }
}

void Proto::start(Type type)
{
    comm_append((uint8_t)type);
    crc.reset();
    crc.byte((uint8_t)type);
}

void Proto::byte(uint8_t data)
{
    comm_append(data_table[(data >> 4) & 0x0F]);
    comm_append(data_table[data & 0x0F]);
    crc.byte(data);
}

void Proto::data(const uint8_t *data, size_t size)
{
    data_with_table(data, size, data_table);
}

void Proto::end()
{
    auto value = crc.value;
    data_with_table((uint8_t*)&value, sizeof(value), crc_table);
    comm_send();
}

size_t Proto::available()
{
    uint32_t free = comm_free();
    uint32_t overhead = HEADER_SIZE + CRC_SIZE;
    if (free < overhead) return 0;
    return (free - overhead) / 2;
}
