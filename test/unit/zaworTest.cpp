
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>

using ::testing::Sequence;
using ::testing::_;

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
static int direction = 0;
static int position = 0;

void global_init()
{

}

void output(int index, bool state)
{
    static bool outputOn = false;
    static bool outputPlus = false;
    if (index == Relay::Index::ZAW_PODL1) outputOn = state;
    if (index == Relay::Index::ZAW_PODL1_PLUS) outputPlus = state;
    direction = !outputOn ? 0 : outputPlus ? +1 : -1;
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

uint32_t getNextTime()
{
    uint32_t next = time + PERIODIC_TIMEOUT;
    if (timeoutSet && timeoutValue > time) {
        next = std::min(next, timeoutValue);
    }
    return next;
}

void setNextTime(uint32_t next)
{
    if (next > time + PERIODIC_TIMEOUT) return;
    if (next <= time) next = time + 1;
    if (!timeoutSet || timeoutValue < time || next < timeoutValue) {
        timeoutSet = true;
        timeoutValue = next;
    }
}

struct MockFunctions {
    MOCK_METHOD(void, outputPeriod, (int dir, uint32_t time), ());
};

MockFunctions* mf = new MockFunctions();

template<typename T>
void updateLoop(const T& updateFunc) {
    bool working;
    uint32_t directionStart = time;
    do {
        int oldDirection = direction;
        Time::update_start();
        working = updateFunc();
        if (direction != oldDirection && time > directionStart) {
            printf("%d for %d\n", oldDirection, time - directionStart);
            mf->outputPeriod(oldDirection, time - directionStart);
            directionStart = time;
        }
        uint32_t next = getNextTime();
        if (timeoutSet && timeoutValue >= time) {
            next = std::min(next, timeoutValue);
        }
        uint32_t delta = next - time;
        time = next;
        if (time <= timeoutValue) {
            timeoutSet = false;
        }
        position += direction * delta;
    } while (working);
    if (time > directionStart) {
        printf("%d for %d\n", direction, time - directionStart);
        mf->outputPeriod(direction, time - directionStart);
    }
}

struct Initializer
{
    Initializer() {
        Storage::init();
    }
} initializer;

Zawor::Storage storageProto {
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

class ZaworTestWithDir : public testing::TestWithParam<int> { };

TEST_P(ZaworTestWithDir, ResetFull) {
    testing::Mock::AllowLeak(mf);
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    Sequence seq;
    int dir = GetParam();

    if (1){
        uint32_t end = time + 2 * 120_sec;
        EXPECT_CALL(*mf, outputPeriod(dir, 135_sec)).Times(1).InSequence(seq);
        EXPECT_CALL(*mf, outputPeriod(0, 300_ms)).Times(1).InSequence(seq);
        EXPECT_CALL(*mf, outputPeriod(-dir, 4_sec)).Times(1).InSequence(seq);
        EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
        zaw.reset(dir, true);
        updateLoop([&]() {
            zaw.update();
            return time < end;
        });
    }
    if (0) {
        uint32_t end = time + 10_sec;
        // EXPECT_CALL(*mf, outputPeriod(-dir, 10_sec)).Times(1).InSequence(seq);
        zaw.reset(-dir, true);
        updateLoop([&]() {
            zaw.update();
            setNextTime(time + 1);
            return time + 1 < end;
        });
        end = time + 2 * 120_sec;
        // EXPECT_CALL(*mf, outputPeriod(0, 300_ms)).Times(1).InSequence(seq);
        // EXPECT_CALL(*mf, outputPeriod(dir, 135_sec)).Times(1).InSequence(seq);
        // EXPECT_CALL(*mf, outputPeriod(0, 300_ms)).Times(1).InSequence(seq);
        // EXPECT_CALL(*mf, outputPeriod(-dir, 4_sec)).Times(1).InSequence(seq);
        // EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
        zaw.reset(dir, true);
        updateLoop([&]() {
            zaw.update();
            return time < end;
        });
    }

}

TEST_P(ZaworTestWithDir, FullSignal) {
    testing::Mock::AllowLeak(mf);
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    Sequence seq;
    int dir = GetParam();
    uint32_t end;

    EXPECT_CALL(*mf, outputPeriod(_, _)).Times(4);
    zaw.reset(-dir, true);
    end = time + 300_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    EXPECT_CALL(*mf, outputPeriod(_, _)).Times(testing::AnyNumber());
    zaw.signal(dir * 256);
    end = time + 80_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
}

INSTANTIATE_TEST_SUITE_P(
    ZaworTestWithDirSuite,
    ZaworTestWithDir,
    ::testing::Values(-1, 1)
);

};