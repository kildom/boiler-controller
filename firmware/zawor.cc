#include  <algorithm>

#include "relays.hh"
#include "time.hh"
#include "storage.hh"
#include "zawor.hh"
#include "log.hh"

// Korutyny dla ubogich:

#define DELAY(time) _DELAY2(time, __LINE__, __COUNTER__);
#define _DELAY2(time, l, c) _DELAY3(time, l, c)
#define _DELAY3(time, l, c) \
    timer.set(time); \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (!timer.ready()) return; \

#define IDLE(cond) _IDLE2(cond, __LINE__, __COUNTER__);
#define _IDLE2(cond, l, c) _IDLE3(cond, l, c)
#define _IDLE3(cond, l, c) \
    resumeLabel = &&_resume_##l##_##c; \
    _resume_##l##_##c: \
    if (cond) return; \


static const int ZAW_RELAY_OFF_TIMEOUT = 300;
static const int ZAW_RESET_WORK_TIME_MUL = 8;


Zawor::Zawor(Storage &storage, Relay::Index relay_on, Relay::Index relay_plus):
    storage(storage),
    relay_on(relay_on),
    relay_plus(relay_plus),
    current_signal(0),
    signal_pos(storage.czas_min_otwarcia),
    real_pos(storage.czas_min_otwarcia),
    totalWorkTime(0),
    event(0),
    direction(0),
    resumeLabel(nullptr),
    lastWorkTime(0),
    delayTime(0)
{
    DBG("%d: Construct, otw %d, min %d", relay_on, storage.czas_otwarcia, storage.czas_min_otwarcia);
}

void Zawor::update()
{
    int threshold = 0;
    bool full = false;
    bool tooLow = false;
    bool tooHigh = false;
    int timeout = 0;
    int margin = 0;
    uint64_t idleTime = 0;

    
    // Pomnóż sygnał przez maksymany stosunek pracy zaworu.
    int signal_adjusted = current_signal * storage.czas_pracy_max / (storage.czas_przerwy + storage.czas_pracy_max);

    // Sygnał nie może wymuszać pracy szybszej niż zawór da radę. Limit to 256, odejmując margines jest 200.
    if (signal_adjusted > 200) {
        signal_adjusted = 200;
    } else if (signal_adjusted < -200) {
        signal_adjusted = 200;
    }

    // Update position based on signal.
    signal_pos += signal_adjusted * Time::delta;

    // Normalizuj jednostki pozycji i limituj do zakresu roboczego.
    int signal_pos_norm = (signal_pos + 128) / 256;
    if (signal_pos_norm < storage.czas_min_otwarcia) {
        signal_pos_norm = storage.czas_min_otwarcia;
        signal_pos = 256 * signal_pos_norm;
    } else if (signal_pos_norm > storage.czas_otwarcia - storage.czas_min_otwarcia) {
        signal_pos_norm = storage.czas_otwarcia - storage.czas_min_otwarcia;
        signal_pos = 256 * signal_pos_norm;
    }

    // Wznów w miejscu ostatniego oczekiwania
    if (resumeLabel != nullptr) {
        goto *resumeLabel;
    }

    if (event == 0) { // Normale sterowanie

        /* Oblicz próg aktywacji zaworu
         *  [oś: threshold]
         *  | 
         *  |\
         *  | \,    __ czas_pracy_max
         *  |  \
         *  |   \_____ czas_pracy_min
         *  |
         *  +------------------------> [oś: idleTime]
         *  |<->| 2 * czas_przerwy
         */
        idleTime = Time::time - lastWorkTime;
        if (idleTime >= 2 * storage.czas_przerwy) {
            threshold = storage.czas_pracy_min;
        } else {
            threshold = 2 * storage.czas_pracy_max - storage.czas_pracy_min - (int)(((int64_t)storage.czas_pracy_max - (int64_t)storage.czas_pracy_min) * (int64_t)idleTime / (int64_t)storage.czas_przerwy);
        }

        // Jeżeli poza progiem, uruchom zawór
        tooLow = real_pos < signal_pos_norm - threshold;
        tooHigh = real_pos > signal_pos_norm + threshold;

        if (tooLow || tooHigh) {
        
            direction = tooLow ? +1 : -1;

            // Uruchom zawór na czas równy różnicy
            set_relays(direction);
            delayTime = direction * (signal_pos_norm - real_pos);
            DELAY(delayTime - storage.korekta);
            set_relays(0);

            // Aktualizuj aktualną pozycję i całkowity czas pracy
            real_pos += direction * (delayTime + timer.overtime());
            totalWorkTime += delayTime;
            lastWorkTime = Time::time;

            // Czekaj na całkowite zatrzymanie
            DELAY(ZAW_RELAY_OFF_TIMEOUT);

            // Jeżeli długi czas pracy bez resetu i znajdujemy się na granicy obszaru roboczego, resetuj pozycję zaworu.
            if (totalWorkTime > ZAW_RESET_WORK_TIME_MUL * storage.czas_otwarcia) {
                if (signal_pos_norm <= storage.czas_min_otwarcia) {
                    event = RESET | FLAG_MINUS;
                    Time::schedule(0);
                } else if (signal_pos_norm >= storage.czas_otwarcia - storage.czas_min_otwarcia) {
                    event = RESET | FLAG_PLUS;
                    Time::schedule(0);
                }
            }
        }

    } else if ((event & 0xF) == RESET) { // Reset zaworu

        // Zapamiętaj parametry eventu i wyczyść event
        direction = (event & FLAG_PLUS) ? +1 : -1;
        full = (event & FLAG_FULL) != 0;
        event = 0;

        // Oblicz czas potrzebny do dojścia do granicy zaworu
        margin = storage.czas_otwarcia / 16;
        if (full) {
            timeout = storage.czas_otwarcia + margin;
        } else if (direction < 0) {
            timeout = real_pos;
        } else {
            timeout = storage.czas_otwarcia - real_pos;
        }
        timeout += margin;

        // Przejdź do brzegu zaworu
        set_relays(direction);
        DELAY(timeout);
        set_relays(0);

        // Czekaj na całkowite zatrzymanie
        DELAY(ZAW_RELAY_OFF_TIMEOUT);

        // Ustaw zawór na minimalne otwarcie/zamknięcie
        set_relays(-direction);
        DELAY(storage.czas_min_otwarcia - storage.korekta);
        set_relays(0);

        // Ustaw zresetowane wartości
        real_pos = storage.czas_min_otwarcia + timer.overtime();
        if (direction > 0) {
            real_pos = storage.czas_otwarcia - real_pos;
        }
        signal_pos = real_pos * 256;
        totalWorkTime = 0;
        current_signal = 0;

        // Czekaj na całkowite zatrzymanie
        DELAY(ZAW_RELAY_OFF_TIMEOUT);
    
    } else if ((event & 0xF) == FORCE) { // Wymuszone działanie
        
        // Zapamiętaj kierunek i czas rozpoczęcia
        direction = (event & FLAG_PLUS) ? +1 : -1;
        lastWorkTime = Time::time;

        // Uruchom zawór tak długo jak długo jest aktywny ten event.
        set_relays(direction);
        IDLE(event == (FORCE | (direction > 0 ? FLAG_PLUS : FLAG_MINUS)));
        set_relays(0);

        // Uaktualnij całkowity czas pracy i rzeczywistą pozycję.
        totalWorkTime += Time::time - lastWorkTime;
        real_pos += direction * (Time::time - lastWorkTime + storage.korekta);
        if (real_pos >= storage.czas_otwarcia) {
            real_pos = storage.czas_otwarcia;
        } else if (real_pos <= 0) {
            real_pos = 0;
        }
        
        // Ustaw pozycję (sygnał) na taką samą jak rzeczywista z wyjątkiem minialnego otwarcia
        signal_pos = real_pos;
        if (signal_pos > storage.czas_otwarcia - storage.czas_min_otwarcia) {
            signal_pos = storage.czas_otwarcia - storage.czas_min_otwarcia;
        } else if (signal_pos < storage.czas_min_otwarcia) {
            signal_pos = storage.czas_min_otwarcia;
        }
        signal_pos *= 256;

        // Czekaj na całkowite zatrzymanie
        DELAY(ZAW_RELAY_OFF_TIMEOUT);
    }

    // Normale wyjście z funkcji powoduje powrót do początku przy następnym wywołaniu.
    resumeLabel = nullptr;
}


void Zawor::reset(int new_direction, bool full)
{
    event = RESET | (new_direction > 0 ? FLAG_PLUS : FLAG_MINUS) | (full ? FLAG_FULL : 0);
    current_signal = 0;
    Time::schedule(0);
}

bool Zawor::ready()
{
    return event == 0 && resumeLabel == nullptr;
}

void Zawor::force(int new_direction)
{
    if (new_direction == 0) {
        event = 0;
    } else  {
        event = FORCE | (new_direction > 0 ? FLAG_PLUS : FLAG_MINUS);
        current_signal = 0;
    }
}

void Zawor::signal(int value)
{
    current_signal = value;
}

bool Zawor::isFullyOpen()
{
    return ((signal_pos + 128) / 256 >= storage.czas_otwarcia - storage.czas_min_otwarcia);
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
