#ifndef _ZAWOR_H_
#define _ZAWOR_H_

#include "global.hh"
#include "relays.hh"
#include "time.hh"

class Zawor
{
public:

    struct Storage {
        // -> BEGIN Zawor::Storage
        // Czas pełnego otwarcia zaworu, time, default: 2_min, range: 1_sec..1_h
        int czas_otwarcia;
        // Czas min. otwarcia zaworu, time, default: 4_sec, range: 0..1_h
        int czas_min_otwarcia;
        // Czas przerwy, time, default: 15_sec, range: 1_sec..10_min
        int czas_przerwy;
        // Czas pracy, time, default: 2_sec, range: 1_sec..10_min
        int czas_pracy_max;
        // Min. dopuszczalny czas pracy., time, default: 1_sec, range: 1_sec..10_min
        int czas_pracy_min;
        // Korekta czasu działania zaw., time, default: 0_sec, range: -1_sec..+1_sec
        int16_t korekta;
        // Temperatura zadana, temp, default: 35_deg, range: 20_deg..80_deg
        int16_t temp;
        // Histereza, temp, default: temp, default: 1_deg, range: 0..10_deg
        int16_t hist;
        // Różnica temp. ster. porporcjonalnego, temp, default: 2_deg, range: 0..20_deg
        int16_t proportionalDiff;
        // Temp. krytyczna, temp, default: 45_deg, range: 0..80_deg
        int critical;
        // -> END
    };

    Storage& storage;
private:

    static const int RESET = 1;
    static const int FORCE = 2;
    static const int FLAG_PLUS = 1 << 8;
    static const int FLAG_MINUS = 0 << 8;
    static const int FLAG_FULL = 1 << 9;

    Relay::Index relay_on;
    Relay::Index relay_plus;

    /// Aktualna wartość sygnału sterującego w zakresie -256...+256 (wyższa wartość dopuszczalna dla szybkiego sterowania).
    int current_signal;
    /// Pozycja zaworu na podstawie sygnału (jednostka: 256 * czas).
    int signal_pos;
    /// Rzeczywista pozycja zaworu (jednostka: czas).
    int real_pos; 
    /// Czas pracy zworu od ostatniego resetu.
    int totalWorkTime;
    /// Aktualne zdarzenie, reset lub wymuszona praca (0 dla normalnego sterowania).
    int event;

    int direction; 
    void* resumeLabel;
    uint64_t lastWorkTime;
    int delayTime;
    Timer timer;

public:

    Zawor(Storage& storage, Relay::Index relay_on, Relay::Index relay_plus);
    void reset(int new_direction, bool full);
    bool ready();
    void force(int new_direction); // is it needed?
    void signal(int value);
    void update();
    bool isFullyOpen();

    static Zawor powrotu;
    static Zawor podl1;
    static Zawor podl2;
    static Zawor* podl[2];

private:

    void set_relays(int new_direction);

};


#endif
