#ifndef __ModelKotlaElekt_HPP
#define __ModelKotlaElekt_HPP


struct ModelKotlaElekt {

    double& T;         // out   Temperatura wyjściowa
    double& Tset;      // param Temperatura zadana
    double& P;         // in    Przepływ przez kocioł
    double& Tin;       // in    Temperatura wejściowa
    bool&   Ron;       // in    Wejście przekaźnika załączającego 0, 1
    bool&   Rpom;      // out   Wyjście przekaźnika pompy 0, 1
    double& CzasStart; // param Czas startu
    double& CzasStop;  // param Czas stopu
    double& Moc;       // param Max. różnica temperatury przy przepływie "1"

    enum {
        OFF,
        STARTING,
        ON,
        STOPPING,
    } state;
    double stateTime;

    ModelKotlaElekt(double& T, double& Tset, double& P, double& Tin, bool& Ron, bool& Rpom, double& CzasStart, double& CzasStop, double& Moc):
        T(T), Tset(Tset), P(P), Tin(Tin), Ron(Ron), Rpom(Rpom), CzasStart(CzasStart), CzasStop(CzasStop), Moc(Moc),
        state(OFF), stateTime(0.0) { }

    template<typename T>
    void setState(T s) {
        stateTime = 0.0;
        state = s;
    }

    void step(double time) {
        stateTime += time;
        T = Tin;
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
            if (Ron) setState(STOPPING);
            break;
        default:
            break;
        }
    }
};


#endif
