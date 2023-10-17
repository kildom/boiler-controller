#ifndef _ZAWOR_H_
#define _ZAWOR_H_

#include "global.hh"
#include "relays.hh"
#include "time.hh"

class Zawor
{
public:

    struct Storage {
        int czas_otwarcia;
        int czas_min_otwarcia;
        int critical;
    };

private:

    enum State {
        DELAY_IDLE = 0,
        IDLE = 1,
        DELAY_PLUS = 2,
        RUN_PLUS = 3,
        RESET_PLUS = 4,
        DELAY_MINUS = 5,
        RUN_MINUS = 6,
        RESET_MINUS = 7,
    };

    Storage& storage;
    Relay::Index relay_on;
    Relay::Index relay_plus;
    int pos;
    int work_time;
    int real_pos;
    State state;
    int8_t direction;
    Timer timer;

public:

    Zawor(Storage& storage, Relay::Index relay_on, Relay::Index relay_plus);
    void reset();
    void full(int new_direction);
    bool full_done();
    void ster(int new_direction);
    void update();
    bool isFull();

    static Zawor powrotu;
    static Zawor podl1;
    static Zawor podl2;

private:

    void set_relays(int new_direction);

};


#endif
