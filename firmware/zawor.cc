#include "relays.hh"
#include "time.hh"
#include "storage.hh"
#include "zawor.hh"
#include "log.hh"

static const int LONG_WORK_TIME = 1000000000;
static const int SMALL_TIME_MARGIN = 10;
static const int ZAW_RELAY_ON_TIMEOUT = 10;
static const int ZAW_RELAY_OFF_TIMEOUT = 100;
static const int ZAW_RESET_WORK_TIME_MUL = 5;
static const int ZAW_RESET_OPEN_TIME_DIV = 8;

Zawor::Zawor(Storage &storage, Relay::Index relay_on, Relay::Index relay_plus):
    storage(storage),
    relay_on(relay_on),
    relay_plus(relay_plus),
    pos(storage.czas_min_otwarcia),
    work_time(LONG_WORK_TIME),
    real_pos(storage.czas_min_otwarcia),
    state(IDLE),
    direction(0)
{
    DBG("%d: Construct, otw %d, min %d", relay_on, storage.czas_otwarcia, storage.czas_min_otwarcia);
}

void Zawor::reset()
{
    real_pos = storage.czas_otwarcia;
    full(-1);
}

void Zawor::full(int new_direction)
{
    direction = 0;
    work_time = LONG_WORK_TIME;
    pos = new_direction < 0 ? storage.czas_min_otwarcia : storage.czas_otwarcia - storage.czas_min_otwarcia;
    Time::schedule(0);
}

bool Zawor::full_done()
{
    bool pos_ok = ((real_pos <= storage.czas_min_otwarcia + SMALL_TIME_MARGIN) && (direction <= 0))
        || ((real_pos >= storage.czas_otwarcia - storage.czas_min_otwarcia - SMALL_TIME_MARGIN) && (direction >= 0));
    return state == IDLE && (pos_ok || work_time < LONG_WORK_TIME);
}

void Zawor::ster(int new_direction)
{
    direction = new_direction < 0 ? -1 : new_direction > 0 ? 1 : 0;
}

void Zawor::update()
{
    // Zmień żądaną pozycję
    pos += direction * Time::delta;
    //DBG("%d: update (%d, %d, %d)", relay_on, direction, pos, real_pos);
    // Ogranicz żądaną pozycję do min max
    if (pos <= storage.czas_min_otwarcia) {
        pos = storage.czas_min_otwarcia;
        if (direction < 0) direction = 0;
    } else if (pos >= storage.czas_otwarcia - storage.czas_min_otwarcia) {
        pos = storage.czas_otwarcia - storage.czas_min_otwarcia;
        if (direction > 0) direction = 0;
    } else if (direction < 0) { // Wymuś update, gdy osiągnięty zostanie limit
        Time::schedule(pos - storage.czas_min_otwarcia);
    } else if (direction > 0) {
        Time::schedule(storage.czas_otwarcia - storage.czas_min_otwarcia - pos);
    }

    switch (state)
    {
    case DELAY_IDLE:
        if (timer.ready()) {
            state = IDLE;
            DBG("%d: DELAY_IDLE => IDLE (%d, %d)", relay_on, pos, real_pos);
        } else {
            break;
        }
        // fallback to IDLE state
    case IDLE:
        if (pos - SMALL_TIME_MARGIN > real_pos) {
            set_relays(+1);
            timer.set(ZAW_RELAY_ON_TIMEOUT);
            state = DELAY_PLUS;
            DBG("%d: IDLE => DELAY_PLUS (%d, %d)", relay_on, pos, real_pos);
        } else if (pos + SMALL_TIME_MARGIN < real_pos) {
            set_relays(-1);
            timer.set(ZAW_RELAY_ON_TIMEOUT);
            state = DELAY_MINUS;
            DBG("%d: IDLE => DELAY_MINUS (%d, %d)", relay_on, pos, real_pos);
        }
        break;

    case DELAY_PLUS:
    case DELAY_MINUS:
        if (timer.ready()) {
            int overtime = timer.overtime();
            real_pos += overtime * (state == DELAY_MINUS ? -1 : 1);
            work_time += overtime;
            state = (State)(state + 1); // RUN_PLUS or RUN_MINUS
            DBG("%d: DELAY_PLUS/MINUS => RUN_PLUS/MINUS (%d, %d)", relay_on, pos, real_pos);
        }
        break;

    case RUN_PLUS:
        real_pos += Time::delta;
        work_time += Time::delta;
        if (real_pos >= pos) {
            if (pos == storage.czas_otwarcia - storage.czas_min_otwarcia && work_time > ZAW_RESET_WORK_TIME_MUL * storage.czas_otwarcia) {
                goto reset_state_entry;
            } else {
                goto delay_idle_state_entry;
            }
        } else if (direction < 0) {
            Time::schedule((pos - real_pos + 1) / 2);
        } else if (direction == 0) {
            Time::schedule(pos - real_pos);
        }
        break;

    case RUN_MINUS:
        real_pos -= Time::delta;
        work_time += Time::delta;
        if (real_pos <= pos) {
            if (pos == storage.czas_min_otwarcia && work_time > ZAW_RESET_WORK_TIME_MUL * storage.czas_otwarcia) {
                goto reset_state_entry;
            } else {
                goto delay_idle_state_entry;
            }
        } else if (direction > 0) {
            Time::schedule((real_pos - pos + 1) / 2);
        } else if (direction == 0) {
            Time::schedule(real_pos - pos);
        }
        break;

    case RESET_PLUS:
    case RESET_MINUS:
        if (timer.ready()) {
            real_pos = (state == RESET_PLUS) ? storage.czas_otwarcia : 0;
            work_time = 0;
            set_relays(0);
            timer.set(ZAW_RELAY_OFF_TIMEOUT);
            state = DELAY_IDLE;
            DBG("%d: RESET_PLUS/MINUS => DELAY_IDLE (%d, %d)", relay_on, pos, real_pos);
        }
        break;

    }

    return;

reset_state_entry:
    DBG("%d: RUN_PLUS/MINUS => RESET_PLUS/MINUS (%d, %d)", relay_on, pos, real_pos);
    timer.set(storage.czas_min_otwarcia + storage.czas_otwarcia / ZAW_RESET_OPEN_TIME_DIV);
    state = (State)(state + 1); // RESET_PLUS or RESET_MINUS
    return;

delay_idle_state_entry:
    DBG("%d: RUN_PLUS/MINUS => DELAY_IDLE (%d, %d)", relay_on, pos, real_pos);
    set_relays(0);
    timer.set(ZAW_RELAY_OFF_TIMEOUT);
    state = DELAY_IDLE;
    return;
}

bool Zawor::isFull()
{
    return real_pos >= storage.czas_otwarcia - storage.czas_min_otwarcia - SMALL_TIME_MARGIN;
}

void Zawor::set_relays(int new_direction)
{
    if (new_direction == 0) {
        Relay::set(relay_on, false);
        Relay::powerOff(relay_plus);
    } else {
        if (new_direction > 0) {
            Relay::set(relay_plus, true);
        } else {
            Relay::set(relay_plus, false);
        }
        Relay::set(relay_on, true);
    }
}

Zawor Zawor::powrotu(::storage->zaw_powrotu, Relay::ZAW_POWR, Relay::ZAW_POWR_PLUS);
Zawor Zawor::podl1(::storage->zaw_podl1, Relay::ZAW_PODL1, Relay::ZAW_PODL1_PLUS);
Zawor Zawor::podl2(::storage->zaw_podl2, Relay::ZAW_PODL2, Relay::ZAW_PODL2_PLUS);
