#ifndef MODEL_HPP
#define MODEL_HPP

#include <math.h>

#include "ModelZaworu.hh"
#include "ModelKotlaElekt.hh"
#include "ModelZasobnika.hh"


struct State {
    // BEGIN STATE

    // Parametry symulacji
    double speed;       // param Prędkość symulacji
    double maxStepSize; // param Maksymalny krok symulacji
    double period;      // param Okres jednego odświerzenia wizualizacji
    bool running;       // calc  Simulation is running

    // Ogólne
    double Time;        // calc  Czas od początku symulacji
    double OpenTime;    // param Czas otwarcia zaworu

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
    double Twyl1;       // mod   Temperatura wylewki 1
    double Tdelta1;     // calc  Jak szybko upływa temperatura z wylewki 1 [°C/s].

    // Podłogówka 2
    double Tpodl2;      // calc  Temperatura podłogówki 2
    double Twyj2;       // mod   Temperatura wyjściowa podłogówki 2
    double P2;          // calc  Przepływ pompy podłogówki 1
    double P2v;         // param Przepływ pompy podłogówki 2, gdy pracuje
    double Z2;          // mod   Zawór podłogówki 2 (0 - niska. temp. podł, 1 - wysoka temp. podł)
    double Z2dir;       // mod   Aktualny kierunek zaworu podłogówki 2
    double Twyl2;       // mod   Temperatura wylewki 2
    double Tdelta2;     // calc  Jak szybko upływa temperatura z wylewki 2 [°C/s]

    // Podłogówka - wspólne
    double PodlA;        // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
    double PodlK;        // param Jak szybko wężownica przekazuje energię do wylewki (jednostki nieznane)
    double Tpoddelta;    // param Jak szybko upływa temperatura z wylewki na 1 °C różnicy [°C/s / °C]

    // Dom
    double Tdom;         // calc  Temperatura domu
    double Tzaddom;      // param Temperatura zadana domu
    double Tzewn;        // param Temperatura na zwenątrz
    double Tdomdelta;    // param Jak szybko zmienia się temperatura domu na 1 °C różnicy z wylewką [°C/s / °C]
    double Tzewndelta;   // param Jak szybko zmienia się temperatura domu na 1 °C różnicy z zewnętrzem [°C/s / °C]

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
    bool   R13;         // out   Relay 13 - buzzer

    // Wejścia
    bool   IN0;         // in    Input 0 - ster. pokojowy
    bool   IN1;         // in    Input 1 - podajnik pelletu

    // END STATE

    double timeoutLeft; // Number of seconds left to the next timeout event

    ModelZaworu modZ0;
    ModelZaworu modZ1;
    ModelZaworu modZ2;
    ModelKotlaElekt elekt;
    ModelZasobnika zas;
    ModelZasobnika podl1Zas;
    ModelZasobnika podl2Zas;

    State() :
        modZ0(Z0, Z0dir, R4, R5, OpenTime),
        modZ1(Z1, Z1dir, R6, R7, OpenTime),
        modZ2(Z2, Z2dir, R9, R10, OpenTime),
        elekt(Tele, Tzadele, P4, Tspz, R2, Rele, EleCzasStart, EleCzasStop, EleMoc),
        zas(Tzas, Tspc, P3, Twyj3, ZasA, ZasK, ZasTwdelta, ZasTwmin),
        podl1Zas(Twyl1, Tpodl1, P1, Twyj1, PodlA, PodlK, Tdelta1, Tdom),
        podl2Zas(Twyl2, Tpodl2, P2, Twyj2, PodlA, PodlK, Tdelta2, Tdom)
    {

        // Parametry symulacji
        speed = 1;       // param Prędkość symulacji
        maxStepSize = 0.05; // param Maksymalny krok symulacji
        period = 0.05;      // param Okres jednego odświerzenia wizualizacji

        // Ogólne
        Time = 0;        // calc  Czas od początku symulacji
        OpenTime = 120;    // param Czas otwarcia zaworu

        // Kocioł na pellet
        Tpowr = 50;       // calc  Temperatura powrotu
        Tpiec = 70;       // mod   Temperatura wyjściowa pieca
        P0 = 0;          // calc  Aktualny przepływ pompki pieca
        P0v = 1;         // param Przepływ pompki pieca, gdy pracuje
        Z0 = 0.4;        // mod   Zawór powrotu pieca (0 - niska temp. powrotu, 1 - wysoka temp. powrotu)
        Z0dir = 0;       // mod   Aktualny kierunek zaworu powrotu pieca

        // Kocioł elektryczny
        Tele = 36;         // mod   Temperatura wyjścia kotła elektrycznego
        Tzadele = 36;      // param Temperatura zadana kotła elektrycznego
        P4 = 0;           // calc  Przepływ pompy kotła elektrycznego
        P4v = 1;          // param Przepływ pompy kotła elektrycznego, gdy pracuje
        Rele = 0;         // mod   Załączenie pompy przez kocioł elekt.
        EleCzasStart = 240; // param Czas startu kotła elektrycznego
        EleCzasStop = 60;  // param Czas zatrzymania kotła elektrycznego
        EleMoc = 4;       // param Max. różnica temperatury przy przepływie "1"

        // Sprzęgło hydrauliczne
        Tspz = 24;        // calc  Temeratura zimnego wyjścia ze sprzęgła
        Tspc = 70;        // calc  Temeratura ciepłego wyjścia ze sprzęgła
        Twejspz = 24;     // calc  Temeratura zimnego wejścia do sprzęgła
        Twejspc = 70;     // calc  Temeratura ciepłego wejścia do sprzęgła

        // Podłogówka 1
        Tpodl1 = 26;      // calc  Temperatura podłogówki 1
        Twyj1 = 26;       // mod   Temperatura wyjściowa podłogówki 1
        P1 = 0;          // calc  Przepływ pompy podłogówki 1
        P1v = 1;         // param Przepływ pompy podłogówki 1, gdy pracuje
        Z1 = 0.31;          // mod   Zawór podłogówki 1 (0 - niska. temp. podł, 1 - wysoka temp. podł)
        Z1dir = 0;       // mod   Aktualny kierunek zaworu podłogówki 1
        Twyl1 = 20;       // mod   Temperatura wylewki 1
        Tdelta1 = 0;     // calc   Jak szybko upływa temperatura z wylewki 1 [°C/s].

        // Podłogówka 2
        Tpodl2 = 26;      // calc  Temperatura podłogówki 2
        Twyj2 = 26;       // mod   Temperatura wyjściowa podłogówki 2
        P2 = 0;          // calc  Przepływ pompy podłogówki 1
        P2v = 1;         // param Przepływ pompy podłogówki 2, gdy pracuje
        Z2 = 0.98;          // mod   Zawór podłogówki 2 (0 - niska. temp. podł, 1 - wysoka temp. podł)
        Z2dir = 0;       // mod   Aktualny kierunek zaworu podłogówki 2
        Twyl2 = 20;       // mod   Temperatura wylewki 1
        Tdelta2 = 0;     // calc  Jak szybko upływa temperatura z wylewki 1 [°C/s].

        // Podłogówka - wspólne
        PodlA = 1;        // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
        PodlK = 0.0004;   // param Jak szybko wężownica przekazuje energię do wylewki (jednostki nieznane)
        Tpoddelta = 0.00007;    // param Jak szybko upływa temperatura z wylewki na 1 °C różnicy [°C/s / °C]

        // Dom
        Tdom = 23;         // calc  Temperatura domu
        Tzaddom = 23;      // param Temperatura zadana domu
        Tzewn = -5;        // param Temperatura na zwenątrz
        Tdomdelta = 0.0001;    // param Jak szybko zmienia się temperatura domu na 1 °C różnicy z wylewką [°C/s / °C]
        Tzewndelta = 0.000023;   // param Jak szybko zmienia się temperatura domu na 1 °C różnicy z zewnętrzem [°C/s / °C]

        // CWU
        Twyj3 = 18;       // mod   Wyjście zasobnika CWU
        Tzas = 10;        // mod   Temperatura wody CWU
        P3 = 0;          // calc  Przepływ pompy CWU
        P3v = 1;         // param Przepływ pompy CWU, gdy pracuje
        ZasA = 0.5;      // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
        ZasK = 0.001;    // param Jak szybko wężownica przekazuje energię do zasobnika (jednostki nieznane)
        ZasTwdelta = 5.0 / 60.0 / 60.0;  // param Jak szybko upływa temperatura ze zbiornika [°C/s].
        ZasTwmin = 8;    // param Minimalna temeratura zasobnika

        // Przekaźniki
        R0 = 0;          // out   Relay 0 - odcięcie paliwa
        R1 = 0;          // out   Relay 1 - włączenie pieca
        R2 = 0;          // out   Relay 2 - włączenie kotła elek.
        R3 = 0;          // out   Relay 3 - pompa pieca
        R4 = 0;          // out   Relay 4 - zawór powrotu on/off
        R5 = 0;          // out   Relay 5 - zawór powrotu +/-
        R6 = 0;          // out   Relay 6 - zawór podl. 1 on/off
        R7 = 0;          // out   Relay 7 - zawór podl. 1 +/-
        R8 = 0;          // out   Relay 8 - pompa podłogówki 1
        R9 = 0;          // out   Relay 9 - zawór podl. 2 on/off
        R10 = 0;         // out   Relay 10 - zawór podl. 2 +/-
        R11 = 0;         // out   Relay 11 - pompa podłogówki 2
        R12 = 0;         // out   Relay 12 - pompa CWU
        R13 = 0;         // out   Relay 13 - buzzer

        // Wejścia
        IN0 = 0;         // in    Input 0 - ster. pokojowy
        IN1 = 0;         // in    Input 1 - podajnik pelletu

        // Inne
        timeoutLeft = 0.0;
    }

    void step(double time)
    {
        P0 = P0v * R3;
        P1 = P1v * R8;
        P2 = P2v * R11;
        P3 = P3v * R12;
        P4 = P4v * Rele;

        modZ0.step(time);
        modZ1.step(time);
        modZ2.step(time);
        elekt.step(time);
        zas.step(time);
        podl1Zas.step(time);
        podl2Zas.step(time);

        Tpowr = (1.0 - Z0) * Tspz + Z0 * Tpiec;
        if (P0 * (1.0 - Z0) + P4 > 0.0) {
            Twejspc = (P0 * (1.0 - Z0) * Tpiec + P4 * Tele) / (P0 * (1.0 - Z0) + P4);
        } else {
            Twejspc = Tspz;
        }

        Tpodl1 = Z1 * Tspc + (1.0 - Z1) * Twyj1;

        Tpodl2 = Z2 * Tspc + (1.0 - Z2) * Twyj2;

        if (P1 * Z1 + P2 * Z2 + P3 > 0.0) {
            Twejspz = (P1 * Z1 * Twyj1 + P2 * Z2 * Twyj2 + P3 * Twyj3) / (P1 * Z1 + P2 * Z2 + P3);
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

        Tdelta1 = (Twyl1 - Tdom) * Tpoddelta;
        Tdelta2 = (Twyl2 - Tdom) * Tpoddelta;
        double Tdompodl = (Twyl1 + Twyl2) * 0.5;
        Tdom += (Tdompodl - Tdom) * Tdomdelta * time;
        Tdom -= (Tdom - Tzewn) * Tzewndelta * time;
        IN0 = Tdom < Tzaddom;

        Time += time;
    }
};

#endif
