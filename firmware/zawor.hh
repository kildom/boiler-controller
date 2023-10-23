#ifndef _ZAWOR_H_
#define _ZAWOR_H_

#include "global.hh"
#include "relays.hh"
#include "time.hh"

class Zawor
{
public:

    struct Storage {
        /// Czas pełnego otwarcia zaworu.
        int czas_otwarcia;
        /// Czas potrzebny na minimalne otwarcie zaworu.
        int czas_min_otwarcia;
        /// Czas przerwy przy maksymanym sygnale sterującym.
        uint16_t czas_przerwy;
        /// Czas pracy przy maksymanym sygnale sterującym.
        uint16_t czas_pracy_max;
        /// Minimalny dopuszczalny czas pracy.
        uint16_t czas_pracy_min;
        /// Czas, o ile dłużej działa zawór niż działa sygnał ze sterownika (ujemny, jeżeli zawór działa krócej).
        int16_t korekta;
        /// Temperatura zadana
        int16_t temp;
        /// Histereza (temp ± hist)
        int16_t hist;
        /// Różnica temp., przy której zaczyna się ster. proporcjonalne
        int16_t proportionalDiff;
        /// Krytyczna temperatura wyjściowa z zaworu.
        int critical;
    };

private:

    static const int RESET = 1;
    static const int FORCE = 2;
    static const int FLAG_PLUS = 1 << 8;
    static const int FLAG_MINUS = 0 << 8;
    static const int FLAG_FULL = 1 << 9;

    Storage& storage;
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
    void force(int new_direction);
    void signal(int value);
    void update();
    bool isFullyOpen();

    static Zawor powrotu;
    static Zawor podl1;
    static Zawor podl2;

private:

    void set_relays(int new_direction);

};


#endif
