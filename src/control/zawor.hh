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
        // Czas pracy, time, default: 2_sec, range: 1_sec..16_sec
        int czas_pracy_max;
        // Min. dopuszczalny czas pracy., time, default: 1_sec, range: 1_sec..16_sec
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

    enum Event {
        INITIAL = 1,
        NORMAL = 2,
        RESET = 3,
        RESET_FULL = 4,
        FORCE_START = 5,
        FORCE_STOP = 6,
        FLAG_PLUS = 1 << 8,
        FLAG_MINUS = 1 << 9,
    };

    Relay::Index relayOn;
    Relay::Index relayPlus;

    /// Aktualna wartość sygnału sterującego w zakresie -256...+256 (wyższa wartość dopuszczalna dla szybkiego sterowania).
    int signalValue;
    /// Pozycja zaworu na podstawie sygnału (jednostka: 256 * czas).
    int signalPos;
    /// Rzeczywista pozycja zaworu (jednostka: czas).
    int realPos;
    /// Czas pracy zworu od ostatniego resetu.
    int totalWorkTime;

    /// Główny timer do sterowania stanem
    Timer timer;
    /// Timer odliczający czas od ostatniego włączenie/wyłączenia siłownika
    Timer switchTimer;
    /// Aktualny kierunek działania (stan przkaźników)
    int realDirection;
    /// Spodziewany kierunek działana, może być inny niż realDirection, jeżeli czekamy na przełączenie przekaźników
    int expectedDirection;

    /// Zminne do przechowywania stanu korutyny
    void* resumeLabel;
    int activeEvent;
    int oldEvent;

public:

    Zawor(Storage& storage, Relay::Index relayOn, Relay::Index relayPlus);
    void reset(int newDirection, bool full);
    bool ready();
    void force(int newDirection);
    void signal(int value);
    void update() { handler(0); };
    bool isFullyOpen();

    static Zawor powrotu;
    static Zawor podl1;
    static Zawor podl2;
    static Zawor* podl[2];

private:
    void handler(int event);
    void sendEvent(int event) { handler(event); }
    bool setDirection(int direction);
    void setRealDirection();
    int getRequestedDir();
    bool posUpdateStop(int direction);
    bool longWorkSyncReq(int direction);

};


#endif
