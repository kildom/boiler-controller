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
    };

    static inline void paliwo(bool on) { output(PALIWO, on); }                 // R0  Relay 0 - odcięcie paliwa
    static inline void piec(bool on) { output(PIEC, on); }                     // R1  Relay 1 - włączenie pieca
    static inline void elek(bool on) { output(ELEK, on); }                     // R2  Relay 2 - włączenie kotła elek.
    static inline void pompa_powr(bool on) { output(POMPA_POWR, on); }         // R3  Relay 3 - pompa pieca
    static inline void zaw_powr(bool on) { output(ZAW_POWR, on); }             // R4  Relay 4 - zawór powrotu on/off
    static inline void zaw_powr_plus(bool on) { output(ZAW_POWR_PLUS, on); }   // R5  Relay 5 - zawór powrotu +/-
    static inline void zaw_podl1(bool on) { output(ZAW_PODL1, on); }           // R6  Relay 6 - zawór podl. 1 on/off
    static inline void zaw_podl1_plus(bool on) { output(ZAW_PODL1_PLUS, on); } // R7  Relay 7 - zawór podl. 1 +/-
    static inline void pompa_podl1(bool on) { output(POMPA_PODL1, on); }       // R8  Relay 8 - pompa podłogówki 1
    static inline void zaw_podl2(bool on) { output(ZAW_PODL2, on); }           // R9  Relay 9 - zawór podl. 2 on/off
    static inline void zaw_podl2_plus(bool on) { output(ZAW_PODL2_PLUS, on); } // R10 Relay 10 - zawór podl. 2 +/-
    static inline void pompa_podl2(bool on) { output(POMPA_PODL2, on); }       // R11 Relay 11 - pompa podłogówki 2
    static inline void pompa_cwu(bool on) { output(POMPA_CWU, on); }           // R12 Relay 12 - pompa CWU
    static inline void buzzer(bool on) { output(BUZZER, on); }                 // R13 Relay 13 - brzęczyk

    static inline void set(Index index, bool on) { output(index, on); }
};

#endif
