#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "global.hh"
#include "zawor.hh"
#include "relays.hh"
#include "temp.hh"

struct Storage {
    uint32_t magic1;
    uint32_t ver;

    Relay::Storage relay;
    Temp::Storage temp;

    // -> BEGIN

    // -> Tryby
    // Pellet C.O., default: false
    bool pelletDom;
    // Pellet C.W.U., default: false
    bool pelletCwu;
    // Elektryczny C.O., default: false
    bool elekDom;
    // Elektryczny C.W.U., default: false
    //bool elekCwu; - always false
    // Elektryczny - C.O. bez zaworów
    //bool elekBezposrPodl; - always true
    // Włącz drugą podłogówkę, default: true
    //bool podl2;

    // -> Konfiguracja -> Zaw. powrotu
    Zawor::Storage zaw_powrotu;
    // -> DEFAULT zaw_powrotu.temp 56_deg
    // -> DEFAULT zaw_powrotu.hist 1_deg
    // -> DEFAULT zaw_powrotu.proportionalDiff 3_deg
    // -> DEFAULT zaw_powrotu.critical 50_deg

    // -> Konfiguracja -> Zaw. podl. 1
    Zawor::Storage zaw_podl1;

    // -> Konfiguracja -> Zaw. podl. 2
    Zawor::Storage zaw_podl2;

    // -> Konfiguracja -> Elektryczny
    // Czas startu, time, default: 1_min, range: 0..30_min
    int elekStartupTime;
    // Temperatura krytyczna, temp, default: 60_deg, range: 30_deg..95_deg
    int16_t elekCritical;
    // Czas wyłączenia, time, default: 1_min, range: 0..30_min
    int elekOffTime;
    // Zalecany min. czas pracy, time, default: 60_min, range: 0..4_h
    int elekMinWorkTime;

    // -> Konfiguracja -> Pellet
    // Max. czas rozgrzewania, time, default: 40_min, range: 0..10_h
    int pelletHeatUpMaxTime;
    // Zalecany min. czas pracy, time, default: 90_min, range: 0..4_h
    int pelletMinWorkTime;
    // Czas do wygaśnięcia ognia, time, default: 5_min, range: 0..1_h
    int pelletCzasWygasania;
    // Min. temp. w pełni rozgrzanego pieca, temp, default: 65_deg, range: 0..80_deg
    int16_t pelletRozgrzanyTemp;
    // Min. czas wył. pomp przy krytycznej temp. powr., time, default: 1_min, range: 0..10_min
    int powrCriticalOffTime;

    // -> Konfiguracja -> C.W.U.
    // Temperatura min., temp, default: 45_deg, range: 0..70_deg
    int16_t cwuTempMin;
    // Temperatura max., temp, default: 55_deg, range: 0..70_deg
    int16_t cwuTempMax;
    // Temperatura krytyczna, temp, default: 65_deg, range: 0..80_deg
    int16_t cwuTempCritical;
    // Dopuszczalne przegrzanie przy studzeniu pieca, temp, default: 5_deg, range: 0..30_deg
    int16_t cwuTempAddCoolDown;
    // Min. różnica temp. przy studzeniu pieca, temp, default: 3_deg, range: 0..30_deg
    int16_t coolDownMarginCwu;
    // Minimalna temp. otoczenia pieca, temp, default: 15_deg, range: 0..30_deg
    int16_t tempOtoczenia;

    // -> Konfiguracja -> Podłogówka
    // Sygnał balansujący podłogówki (%), int, default: 10, range: 0..50
    int16_t sygnalBalansujacy;

    // -> Konfiguracja -> Podłogówka -> Wykrywanie błędów
    // Czas na wykrycie uszkodzenia pompy, time, default: 10_min, range: 0..10_h
    int podlFaultDelay;
    // Min. temp. pieca przy wykryciu uszkodzenia pompy, temp, default: 50_deg, range: 0..70_deg
    int16_t podlFaultPiecTemp;
    // Max. czas ponownego rozpalania pieca, time, default: 1_h, range: 0..10_h
    int pelletMaxCzasPostoju;

    // -> END

    uint32_t crc;
    uint32_t magic2;
    uint32_t _persistent_end;
    // ---------------- Non-permanent data
    uint64_t time;
    bool cwuHeatState;
    static void init();
    static void write();
    static void update();
private:
    static uint32_t getCrc();
};

extern Storage storage;

#endif
