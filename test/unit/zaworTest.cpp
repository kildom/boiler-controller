
#include <gtest/gtest.h>
#include <algorithm>
#include <iostream>

namespace zaworTests {

#define GLOBAL ::zaworTests

//#define _RELAYS_HH_
//#define _TIMER_HH_
//#define _STORAGE_H_
//#define _LOG_H_

#include "zawor.hh"
#include "zawor.cc"

#include "time.cc"
#include "relays.cc"
#include "storage.cc"
#include "log.cc"
#include "deque.cc"
#include "crc.cc"

static uint32_t time = 0;
static bool timeoutSet = false;
static uint32_t timeoutValue = 0;

void global_init()
{

}

void output(int index, bool state)
{
    static uint32_t last = 0;
    printf("%d [%f]: %d = %s\n", time, double(time - last) / 1000.0, index, state ? "true" : "false");
    last = time;
}

bool input(int index); // ON/OFF with pull-up
uint32_t analog_input(int index); // Raw 16-bit value of analog input, converting and validating is done by upper level

uint32_t get_time()
{
    return time;
}

void timeout(uint32_t t)
{
    timeoutSet = true;
    timeoutValue = t;
}

void store_read(int slot, uint8_t* buffer, int size)
{
    memset(buffer, 0, size);
}

void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size)
{
    *state = 0;
}

int comm_free(); // Free space in communication FIFO
void comm_append(uint8_t data); // Append data to FIFO
void comm_send(); // Send appended data, just raw data, SLIP encoding in upper level

int diag_free(); // Free space in diagnostics FIFO
void diag_append(uint8_t data); // Append data to FIFO
void diag_send(); // Send appended data, just raw data
void diag_log(int level, const char *message)
{
    switch (level) {
        case 0:
            std::cerr << "DEBUG: " << message << std::endl;
            break;
        case 1:
            std::cerr << "WARNING: " << message << std::endl;;
            break;
        default:
            std::cerr << "ERROR: " << message << std::endl;
            break;
    }
}


//void startup_event(); // main() function is in lower level
//void timeout_event();
//void comm_event(uint8_t* data, int size); // just raw data, SLIP decoding/validating/synch in upper level
//void diag_event(uint8_t* data, int size); // just raw data

template<typename T>
void updateLoop(uint32_t endTime, const T& updateFunc) {
    while (time < endTime) {
        uint32_t next = std::min(time + PERIODIC_TIMEOUT, endTime);
        if (timeoutSet && timeoutValue >= time) {
            next = std::min(next, timeoutValue);
        }
        time = next;
        if (time <= timeoutValue) {
            timeoutSet = false;
        }
        Time::update_start();
        updateFunc();
    }
}

TEST(ZaworTest, Reset) {
    Storage::init();
    Zawor::Storage storage {
        // Czas pełnego otwarcia zaworu, time, default: 2_min, range: 1_sec..1_h
        .czas_otwarcia = 2_min,
        // Czas min. otwarcia zaworu, time, default: 4_sec, range: 0..1_h
        .czas_min_otwarcia = 4_sec,
        // Czas przerwy, time, default: 15_sec, range: 1_sec..10_min
        .czas_przerwy = 15_sec,
        // Czas pracy, time, default: 2_sec, range: 1_sec..10_min
        .czas_pracy_max = 2_sec,
        // Min. dopuszczalny czas pracy., time, default: 1_sec, range: 1_sec..10_min
        .czas_pracy_min = 1_sec,
        // Korekta czasu działania zaw., time, default: 0_sec, range: -1_sec..+1_sec
        .korekta = 0_sec,
        // Temperatura zadana, temp, default: 35_deg, range: 20_deg..80_deg
        .temp = 35_deg,
        // Histereza, temp, default: temp, default: 1_deg, range: 0..10_deg
        .hist = 1_deg,
        // Różnica temp. ster. porporcjonalnego, temp, default: 2_deg, range: 0..20_deg
        .proportionalDiff = 2_deg,
        // Temp. krytyczna, temp, default: 45_deg, range: 0..80_deg
        .critical = 50_deg,
    };
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    zaw.reset(-1, true);
    updateLoop(2 * storage.czas_otwarcia, [&zaw]() {
        zaw.update();
    });
    zaw.signal(200);
    updateLoop(time + 2 * storage.czas_otwarcia, [&zaw]() {
        zaw.update();
    });
}



};