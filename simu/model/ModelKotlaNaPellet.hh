#ifndef __ModelKotlaNaPellet_HPP
#define __ModelKotlaNaPellet_HPP

#include <algorithm>

#include "modelCommon.hh"

struct ModelKotlaNaPellet {

    fptype& T;         // out   Temperatura wyjściowa
    fptype& Tset;      // param Temperatura zadana
    fptype& P;         // in    Przepływ przez kocioł
    fptype& Tin;       // in    Temperatura wejściowa
    bool&   Ron;       // in    Wejście przekaźnika załączającego
    bool&   Rzalpal;   // in    Wejście zalaczajace dojście paliwa
    bool&   Rpom;      // out   Wyjście przekaźnika pompy
    bool&   Rpaliwo;   // out   Wyjście przekaźnika podajnika pelletu
    fptype& Tpom;      // param Temperatura załączenia pompy
    fptype& CzasStart; // param Czas startu
    fptype& CzasStop;  // param Czas stopu
    fptype& Moc;       // param Max. różnica temperatury przy przepływie "1"
    fptype& CzasPrzej; // param Czas przejscia wody przez cały kocioł przy przepływie "1"
    fptype& Utrata;    // param Utrata ciepła (°C/s)/°C

    static constexpr fptype MarginesPrzegrzania = 2.0f;
    static constexpr fptype ZimnaTemp = 20.0f;
    static constexpr fptype CzasPrzedPodsypem = 60.0f;
    static constexpr fptype CzasPodsypu = 15.0f;
    static constexpr fptype CzasMinPrzerwyPaliwa = 15.0f;
    static constexpr fptype CzasPodawaniaPaliwa = 5.0f;
    static constexpr int count = 16;

    enum State {
        OFF,
        STARTING,
        ON,
        FAIL,
    } state;

    fptype stateTime;
    fptype zbiorniki[count];
    fptype czasPaliwa;
    fptype czasBrakuPaliwa;
    fptype faza;
    fptype akumulatorWejscaSum;
    fptype akumulatorWejscaTime;

    ModelKotlaNaPellet(
        fptype& T,
        fptype& Tset,
        fptype& P,
        fptype& Tin,
        bool&   Ron,
        bool&   Rzalpal,
        bool&   Rpom,
        bool&   Rpaliwo,
        fptype& Tpom,
        fptype& CzasStart,
        fptype& CzasStop,
        fptype& Moc,
        fptype& CzasPrzej,
        fptype& Utrata
    ):
        T(T),
        Tset(Tset),
        P(P),
        Tin(Tin),
        Ron(Ron),
        Rzalpal(Rzalpal),
        Rpom(Rpom),
        Rpaliwo(Rpaliwo),
        Tpom(Tpom),
        CzasStart(CzasStart),
        CzasStop(CzasStop),
        Moc(Moc),
        CzasPrzej(CzasPrzej),
        Utrata(Utrata),
        stateTime(10000000.0_f),
        czasPaliwa(0.0_f),
        czasBrakuPaliwa(0.0_f),
        faza(0.0_f),
        akumulatorWejscaSum(0.0_f),
        akumulatorWejscaTime(0.0_f)
    {
        for (int i = 0; i < count; i++) {
            zbiorniki[i] = ZimnaTemp;
        }
    }

    void setState(State s) {
        stateTime = 0.0_f;
        state = s;
        if (state == ON) czasBrakuPaliwa = 0.0_f;
    }

    fptype heat(fptype time, fptype power) {
        fptype wzrost = Moc * time / CzasPrzej;
        fptype wzrostRzeczywisty = 0.0_f;
        for (int i = 0; i < count; i++) {
            fptype utrata = Utrata * (zbiorniki[0] - ZimnaTemp) * time;
            zbiorniki[i] -= utrata;
            fptype wzrostMax = std::min(power * wzrost, Tset + MarginesPrzegrzania - zbiorniki[i]);
            wzrostRzeczywisty += wzrostMax;
            zbiorniki[i] += wzrostMax;
        }
        wzrostRzeczywisty /= (fptype)count;
        return wzrostRzeczywisty / wzrost;
    }

    void step(fptype time) {
        stateTime += time;

        fptype zmianaFazy = time / CzasPrzej * fptype(count) * P;
        faza += zmianaFazy;
        akumulatorWejscaSum += Tin * zmianaFazy;
        akumulatorWejscaTime += zmianaFazy;
        if (faza >= 1.0_f) {
            faza -= 1.0_f;
            for (int i = count - 1; i > 0; i--) {
                zbiorniki[i] = zbiorniki[i - 1];
            }
            zbiorniki[0] = akumulatorWejscaSum / akumulatorWejscaTime;
            akumulatorWejscaSum = 0.0_f;
            akumulatorWejscaTime = 0.0_f;
            for (int i = 0; i < count - 1 && zbiorniki[i] > zbiorniki[i + 1]; i++) {
                std::swap(zbiorniki[i], zbiorniki[i + 1]);
            }
        }

        switch (state)
        {
        case FAIL:
        case OFF:
            if (stateTime < CzasStop) {
                heat(time, 1.0_f - (stateTime / CzasStop));
            }
            Rpaliwo = false;
            if (state == OFF && Ron) setState(STARTING);
            break;

        case STARTING:
            Rpaliwo = stateTime > CzasPrzedPodsypem && stateTime < CzasPrzedPodsypem + CzasPodsypu;
            if (Rpaliwo && !Rzalpal) {
                setState(FAIL);
            }
            if (!Ron) setState(OFF);
            if (stateTime > std::max(CzasStart, CzasPrzedPodsypem + CzasPodsypu)) setState(ON);
            break;

        case ON: {
            auto power = heat(time, 1.0_f);
            czasPaliwa += time;
            if (power > 0.001_f && czasPaliwa > (CzasMinPrzerwyPaliwa + CzasPodawaniaPaliwa) / power) {
                czasPaliwa = 0;
            }
            Rpaliwo = czasPaliwa < CzasPodawaniaPaliwa;
            if (Rpaliwo && !Rzalpal) {
                czasBrakuPaliwa += time;
                if (czasBrakuPaliwa > 5 * CzasPodawaniaPaliwa) {
                    setState(FAIL);
                }
            } else {
                czasBrakuPaliwa = 0;
            }
            if (!Ron) setState(OFF);
            break;
        }
        }

        T = zbiorniki[count - 2] * faza + zbiorniki[count - 1] * (1.0_f - faza);
        Rpom = T > Tpom;
    }
};


#endif
