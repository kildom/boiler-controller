
#include <math.h>

#include "ModelZaworu.hpp"
#include "ModelKotlaElekt.hpp"
#include "ModelZasobnika.hpp"


struct State {
    // BEGIN STATE

    // Kocioł na pellet
    double Tpowr;       // calc  Temperatura powrotu
    double Tpiec;       // mod   Temperatura wyjściowa pieca
    double P0;          // calc  Aktualny przepływ pompki pieca
    double P0v;         // param Przepływ pompki pieca, gdy pracuje
    double Z0;          // mod   Zawór powrotu pieca (0 - niska temp. powrotu, 1 - wysoka temp. powrotu)
    double Z0dir;       // mod   Aktualny kierunek zaworu powrotu pieca

    // Kocioł elektryczny
    double Tele;         // mod   Temperatura wyjścia kotła elektrycznego
    double Tzadele;      // param Temperatura zadana kotła elektrycznego
    double P4;           // calc  Przepływ pompy kotła elektrycznego
    double P4v;          // param Przepływ pompy kotła elektrycznego, gdy pracuje
    bool   Rele;         // mod   Załączenie pompy przez kocioł elekt.
    double EleCzasStart; // param Czas startu kotła elektrycznego
    double EleCzasStop;  // param Czas zatrzymania kotła elektrycznego
    double EleMoc;       // param Max. różnica temperatury przy przepływie "1"

    // Sprzęgło hydrauliczne
    double Tspz;        // calc  Temeratura zimnego wyjścia ze sprzęgła
    double Tspc;        // calc  Temeratura ciepłego wyjścia ze sprzęgła
    double Twejspz;     // calc  Temeratura zimnego wejścia do sprzęgła
    double Twejspc;     // calc  Temeratura ciepłego wejścia do sprzęgła

    // Podłogówka 1
    double Tpodl1;      // calc  Temperatura podłogówki 1
    double Twyj1;       // mod   Temperatura wyjściowa podłogówki 1
    double P1;          // calc  Przepływ pompy podłogówki 1
    double P1v;         // param Przepływ pompy podłogówki 1, gdy pracuje
    double Z1;          // mod   Zawór podłogówki 1 (0 - niska. temp. podł, 1 - wysoka temp. podł)
    double Z1dir;       // mod   Aktualny kierunek zaworu podłogówki 1

    // Podłogówka 2
    double Tpodl2;      // calc  Temperatura podłogówki 2
    double Twyj2;       // mod   Temperatura wyjściowa podłogówki 2
    double P2;          // calc  Przepływ pompy podłogówki 1
    double P2v;         // param Przepływ pompy podłogówki 2, gdy pracuje
    double Z2;          // mod   Zawór podłogówki 2 (0 - niska. temp. podł, 1 - wysoka temp. podł)
    double Z2dir;       // mod   Aktualny kierunek zaworu podłogówki 2

    // CWU
    double Twyj3;       // mod   Wyjście zasobnika CWU
    double Tzas;        // mod   Temperatura wody CWU
    double P3;          // calc  Przepływ pompy CWU
    double P3v;         // param Przepływ pompy CWU, gdy pracuje
    double ZasA;        // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
    double ZasK;        // param Jak szybko wężownica przekazuje energię do zasobnika (jednostki nieznane)
    double ZasTwdelta;  // param Jak szybko upływa temperatura ze zbiornika [°C/s].
    double ZasTwmin;    // param Minimalna temeratura zasobnika

    // Przekaźniki
    bool   R0;          // out   Relay 0 - odcięcie paliwa
    bool   R1;          // out   Relay 1 - włączenie pieca
    bool   R2;          // out   Relay 2 - włączenie kotła elek.
    bool   R3;          // out   Relay 3 - pompa pieca
    bool   R4;          // out   Relay 4 - zawór powrotu on/off
    bool   R5;          // out   Relay 5 - zawór powrotu +/-
    bool   R6;          // out   Relay 6 - zawór podl. 1 on/off
    bool   R7;          // out   Relay 7 - zawór podl. 1 +/-
    bool   R8;          // out   Relay 8 - pompa podłogówki 1
    bool   R9;          // out   Relay 9 - zawór podl. 2 on/off
    bool   R10;         // out   Relay 10 - zawór podl. 2 +/-
    bool   R11;         // out   Relay 11 - pompa podłogówki 2
    bool   R12;         // out   Relay 12 - pompa CWU

    // Wejścia
    bool   IN0;         // in    Input 0 - tryb zima
    bool   IN1;         // in    Input 1 - wybrany kocioł elektryczny
    bool   IN2;         // in    Input 2 - ster. pokojowy

    // Ogólne
    double OpenTime;    // param Czas otwarcia zaworu
    double Czas;        // param Czas od początku symulacji

    // END STATE

    ModelZaworu modZ0;
    ModelZaworu modZ1;
    ModelZaworu modZ2;
    ModelKotlaElekt elekt;
    ModelZasobnika zas;

    State() :
        modZ0(Z0, Z0dir, R4, R5, OpenTime),
        modZ1(Z1, Z1dir, R6, R7, OpenTime),
        modZ2(Z2, Z2dir, R9, R10, OpenTime),
        elekt(Tele, Tzadele, P4, Tspz, R2, Rele, EleCzasStart, EleCzasStop, EleMoc),
        zas(Tzas, Tspc, P3, Twyj3, ZasA, ZasK, ZasTwdelta, ZasTwmin)
    {
    }

    void step(double time)
    {
        P0 = P0v * R3;
        P1 = P1v * R8;
        P2 = P2v * R11;
        P3 = P3v * R12;
        P4 = P4v * Rele;
        Tpiec = 67.2; // TODO: model
        Tele = 29.2; // TODO: model
        Tpowr = (1.0 - Z0) * Tspz + Z0 * Tpiec;
        if (P0 * (1.0 - Z0) + P4 > 0.0) {
            Twejspc = (P0 * (1.0 - Z0) * Tpiec + P4 * Tele) / (P0 * (1.0 - Z0) + P4);
        } else {
            Twejspc = (Tpiec + Tele) / 2.0;
        }

        Twyj1 = 24.2; // TODO: model
        Tpodl1 = Z1 * Tspc + (1.0 - Z1) * Twyj1;

        Twyj2 = 23.8; // TODO: model
        Tpodl2 = Z2 * Tspc + (1.0 - Z2) * Twyj2;

        Twyj3 = 20.0; // TODO: model

        if (P1 * Z1 + P2 * Z2 + P3 > 0.0) {
            Twejspz = (P1 * Z1 * Twyj1 + P2 * Z2 + Twyj2 + P3 * Twyj3) / (P1 * Z1 + P2 * Z2 + P3);
        } else {
            Twejspz = (Twyj1 + Twyj2 + Twyj3) / 3.0;
        }

        double Pp = (1 - Z0) * P0 + P4;
        double Ps = Z1 * P1 + Z2 * P2 + P3;
        if (Pp > Ps) {
            double Pd = Pp - Ps;
            Tspc = Twejspc;
            Tspz = (Pd * Twejspc + Ps * Twejspz) / (Pd + Ps);
        } else if (Pp < Ps) {
            double Pd = Ps - Pp;
            Tspz = Twejspz;
            Tspc = (Pd * Twejspz + Pp * Twejspc) / (Pd + Pp);
        } else {
            Tspz = Twejspz;
            Tspc = Twejspc;
        }

        Czas += time;

    }
};
