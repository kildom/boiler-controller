#ifndef _RELAYS_HH_
#define _RELAYS_HH_

#include "global.hh"
#include "lowlevel.hh"

class Relay {
public:

    enum Index {
        PALIWO = 0,
        PIEC = 1,
        ELEK = 2,
        POMPA_POWR = 3,
        ZAW_POWR = 4,
        ZAW_POWR_PLUS = 5,
        ZAW_PODL1 = 6,
        ZAW_PODL1_PLUS = 7,
        POMPA_PODL1 = 8,
        ZAW_PODL2 = 9,
        ZAW_PODL2_PLUS = 10,
        POMPA_PODL2 = 11,
        POMPA_CWU = 12,
        BUZZER = 13,
        OUTPUTS_COUNT = 14,
    };

    struct Storage {
        uint8_t map[OUTPUTS_COUNT];
        uint32_t invert; // low level indexing (without mapping)
    };

    static uint32_t state; // low level state (without inverting or mapping)

    static inline void paliwo(bool on) { set(PALIWO, on); }                 // R0  Relay 0 - odcięcie paliwa
    static inline bool paliwo() { return get(PALIWO); }
    static inline void piec(bool on) { set(PIEC, on); }                     // R1  Relay 1 - włączenie pieca
    static inline bool piec() { return get(PIEC); }
    static inline void elek(bool on) { set(ELEK, on); }                     // R2  Relay 2 - włączenie kotła elek.
    static inline bool elek() { return get(ELEK); }
    static inline void pompa_powr(bool on) { set(POMPA_POWR, on); }         // R3  Relay 3 - pompa pieca
    static inline bool pompa_powr() { return get(POMPA_POWR); }
    static inline void zaw_powr(bool on) { set(ZAW_POWR, on); }             // R4  Relay 4 - zawór powrotu on/off
    static inline bool zaw_powr() { return get(ZAW_POWR); }
    static inline void zaw_powr_plus(bool on) { set(ZAW_POWR_PLUS, on); }   // R5  Relay 5 - zawór powrotu +/-
    static inline bool zaw_powr_plus() { return get(ZAW_POWR_PLUS); }
    static inline void zaw_podl1(bool on) { set(ZAW_PODL1, on); }           // R6  Relay 6 - zawór podl. 1 on/off
    static inline bool zaw_podl1() { return get(ZAW_PODL1); }
    static inline void zaw_podl1_plus(bool on) { set(ZAW_PODL1_PLUS, on); } // R7  Relay 7 - zawór podl. 1 +/-
    static inline bool zaw_podl1_plus() { return get(ZAW_PODL1_PLUS); }
    static inline void pompa_podl1(bool on) { set(POMPA_PODL1, on); }       // R8  Relay 8 - pompa podłogówki 1
    static inline bool pompa_podl1() { return get(POMPA_PODL1); }
    static inline void zaw_podl2(bool on) { set(ZAW_PODL2, on); }           // R9  Relay 9 - zawór podl. 2 on/off
    static inline bool zaw_podl2() { return get(ZAW_PODL2); }
    static inline void zaw_podl2_plus(bool on) { set(ZAW_PODL2_PLUS, on); } // R10 Relay 10 - zawór podl. 2 +/-
    static inline bool zaw_podl2_plus() { return get(ZAW_PODL2_PLUS); }
    static inline void pompa_podl2(bool on) { set(POMPA_PODL2, on); }       // R11 Relay 11 - pompa podłogówki 2
    static inline bool pompa_podl2() { return get(POMPA_PODL2); }
    static inline void pompa_cwu(bool on) { set(POMPA_CWU, on); }           // R12 Relay 12 - pompa CWU
    static inline bool pompa_cwu() { return get(POMPA_CWU); }
    static inline void buzzer(bool on) { set(BUZZER, on); }                 // R13 Relay 13 - brzęczyk
    static inline bool buzzer() { return get(BUZZER); }

    static void set(Index index, bool on);
    static bool get(Index index);
    static void powerOff(Index index);
};

#endif
