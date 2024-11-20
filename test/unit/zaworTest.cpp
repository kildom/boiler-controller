
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>

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

void timeout(uint32_t t)
{
    printf("timeout in %d\n", t - time);
    setNextTime(t);
}


struct MockFunctions {
    bool outputPeriodEnable;
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
            printf("######## %d for %d\n", oldDirection, time - directionStart);
            if (mf->outputPeriodEnable)
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
        printf("######## %d for %d\n", direction, time - directionStart);
        if (mf->outputPeriodEnable)
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
    testing::Sequence seq;
    int dir = GetParam();
    mf->outputPeriodEnable = true;

    // Prosty reset
    uint32_t end = time + 2 * 120_sec;
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(testing::AtMost(1)).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, 135_sec)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, 500_ms)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, 4_sec)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.reset(dir, true);
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    // Przerwany reset przez reset w drugą stronę
    mf->outputPeriodEnable = false;
    zaw.reset(-dir, true);
    end = time + 10_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
    mf->outputPeriodEnable = true;
    EXPECT_CALL(*mf, outputPeriod(0, 500_ms)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, 135_sec)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, 500_ms)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, 4_sec)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.reset(dir, true);
    end = time + 2 * 120_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    testing::Mock::VerifyAndClearExpectations(mf);
    testing::Mock::AllowLeak(mf);
}

template <typename T>
auto allowError(T x, float percentage) {
    T diff = (x * (T)percentage) / (T)100;
    return testing::AllOf(testing::Ge(x - diff), testing::Le(x + diff));
}

TEST_P(ZaworTestWithDir, FullSignal) {
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    testing::Sequence seq;
    int dir = GetParam();
    uint32_t end;
    mf->outputPeriodEnable = false;

    zaw.reset(-dir, true);
    updateLoop([&]() {
        zaw.update();
        return !zaw.ready();
    });

    mf->outputPeriodEnable = true;
    EXPECT_CALL(*mf, outputPeriod(0, allowError(15000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(15000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(15000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(dir * 256);
    end = time + 58_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
    
    printf("-------\n");
    EXPECT_CALL(*mf, outputPeriod(0, testing::AllOf(testing::Ge(15000), testing::Le(22500)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(15000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(15000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(2000, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(_, _)).Times(testing::AnyNumber()).InSequence(seq);
    zaw.signal(-dir * 256);
    end = time + 65_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    testing::Mock::VerifyAndClearExpectations(mf);
    testing::Mock::AllowLeak(mf);
}

TEST_P(ZaworTestWithDir, HalfSignal) {
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    testing::Sequence seq;
    int dir = GetParam();
    uint32_t end;
    mf->outputPeriodEnable = false;

    zaw.reset(-dir, true);
    updateLoop([&]() {
        zaw.update();
        return !zaw.ready();
    });

    printf("-------\n");
    mf->outputPeriodEnable = true;
    EXPECT_CALL(*mf, outputPeriod(0, allowError(32_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(32_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(32_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(2_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(dir * 128);
    end = time + 105_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
    
    printf("-------\n");
    EXPECT_CALL(*mf, outputPeriod(0, testing::AllOf(testing::Ge(15_sec), testing::Le(33_sec)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, testing::AllOf(testing::Ge(1_sec), testing::Le(2_sec)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(32_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(2_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(32_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(2_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(-dir * 128);
    end = time + 115_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    testing::Mock::VerifyAndClearExpectations(mf);
    testing::Mock::AllowLeak(mf);
}

TEST_P(ZaworTestWithDir, QuatreSignal) {
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    testing::Sequence seq;
    int dir = GetParam();
    uint32_t end;
    mf->outputPeriodEnable = false;

    zaw.reset(-dir, true);
    updateLoop([&]() {
        zaw.update();
        return !zaw.ready();
    });

    printf("-------\n");
    mf->outputPeriodEnable = true;
    EXPECT_CALL(*mf, outputPeriod(0, allowError(33_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(33_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(33_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(dir * 64);
    end = time + 105_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
    
    printf("-------\n");
    EXPECT_CALL(*mf, outputPeriod(0, testing::AllOf(testing::Ge(15_sec), testing::Le(40_sec)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, testing::AllOf(testing::Ge(900), testing::Le(1100)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(33_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(33_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(-dir * 64);
    end = time + 200_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    testing::Mock::VerifyAndClearExpectations(mf);
    testing::Mock::AllowLeak(mf);
}

TEST_P(ZaworTestWithDir, SmallSignal) {
    Zawor::Storage storage(storageProto);
    Zawor zaw(storage, Relay::Index::ZAW_PODL1, Relay::Index::ZAW_PODL1_PLUS);
    testing::Sequence seq;
    int dir = GetParam();
    uint32_t end;
    mf->outputPeriodEnable = false;

    zaw.reset(-dir, true);
    updateLoop([&]() {
        zaw.update();
        return !zaw.ready();
    });

    printf("-------\n");
    mf->outputPeriodEnable = true;
    EXPECT_CALL(*mf, outputPeriod(0, allowError(67_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(67_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(67_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(dir * 32);
    end = time + 224_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });
    
    printf("-------\n");
    EXPECT_CALL(*mf, outputPeriod(0, testing::AllOf(testing::Ge(15_sec), testing::Le(100_sec)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, testing::AllOf(testing::Ge(900), testing::Le(1100)))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(67_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, allowError(67_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(-dir, allowError(1_sec, 1))).Times(1).InSequence(seq);
    EXPECT_CALL(*mf, outputPeriod(0, _)).Times(1).InSequence(seq);
    zaw.signal(-dir * 32);
    end = time + 400_sec;
    updateLoop([&]() {
        zaw.update();
        return time < end;
    });

    testing::Mock::VerifyAndClearExpectations(mf);
    testing::Mock::AllowLeak(mf);
}

INSTANTIATE_TEST_SUITE_P(
    ZaworTestWithDirSuite,
    ZaworTestWithDir,
    ::testing::Values(-1, 1)
);

};