#ifndef __ModelKotlaElekt_HPP
#define __ModelKotlaElekt_HPP


#include "modelCommon.hh"

struct ModelKotlaElekt {

    fptype& T;         // out   Temperatura wyjściowa
    fptype& Tset;      // param Temperatura zadana
    fptype& P;         // in    Przepływ przez kocioł
    fptype& Tin;       // in    Temperatura wejściowa
    bool&   Ron;       // in    Wejście przekaźnika załączającego 0, 1
    bool&   Rpom;      // out   Wyjście przekaźnika pompy 0, 1
    fptype& CzasStart; // param Czas startu
    fptype& CzasStop;  // param Czas stopu
    fptype& Moc;       // param Max. różnica temperatury przy przepływie "1"

    enum {
        OFF,
        STARTING,
        ON,
        STOPPING,
    } state;
    fptype stateTime;

    ModelKotlaElekt(fptype& T, fptype& Tset, fptype& P, fptype& Tin, bool& Ron, bool& Rpom, fptype& CzasStart, fptype& CzasStop, fptype& Moc):
        T(T), Tset(Tset), P(P), Tin(Tin), Ron(Ron), Rpom(Rpom), CzasStart(CzasStart), CzasStop(CzasStop), Moc(Moc),
        state(OFF), stateTime(0.0) { }

    template<typename T>
    void setState(T s) {
        stateTime = 0.0_f;
        state = s;
    }

    void step(fptype time) {
        stateTime += time;
        T = Rpom ? Tin : 20.0_f;
        switch (state)
        {
        case OFF:
            Rpom = false;
            if (Ron) setState(STARTING);
            break;
        case STARTING:
            Rpom = true;
            if (stateTime >= CzasStart) setState(ON);
            if (!Ron) setState(STOPPING);
            break;
        case STOPPING:
            Rpom = true;
            if (stateTime >= CzasStop) setState(OFF);
            if (Ron) setState(STARTING);
            break;
        case ON:
            if (P > 0) {
                T += Moc / P;
            } else {
                T += 100;
            }
            if (T > Tset) {
                T = Tset;
                if (T < Tin) {
                    T = Tin;
                }
            }
            Rpom = true;
            if (!Ron) setState(STOPPING);
            break;
        default:
            break;
        }
    }
};


#endif
