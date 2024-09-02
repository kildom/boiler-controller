#ifndef __ModelZasobnika_HPP
#define __ModelZasobnika_HPP

#include <math.h>

#include "modelCommon.hh"

struct ModelZasobnika {

    fptype& T;       // out   Temperatura wody w zasobniku
    fptype& Tin;     // in    Temperatura wejściowa
    fptype& P;       // in    Przepływ wody w wężownicy
    fptype& Tout;    // out   Temperatura wyjściowa
    fptype& A;       // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
    fptype& K;       // param Jak szybko wężownica przekazuje energię do zasobnika (jednostki nieznane)
    fptype& Twdelta; // param Jak szybko upływa temperatura ze zbiornika [°C/s].
    fptype& Twmin;   // param Minimalna temeratura zasobnika

    ModelZasobnika(fptype& T, fptype& Tin, fptype& P, fptype& Tout, fptype& A, fptype& K, fptype& Twdelta, fptype& Twmin):
        T(T), Tin(Tin), P(P), Tout(Tout), A(A), K(K), Twdelta(Twdelta), Twmin(Twmin) { }

    void step(fptype time)
    {
        if (P > 0.0_f) {
            fptype Ap = A / P;
            // założenie: rozkład temp. na wężownicy opisuje równanie y'(x) = (T-y) * Ap, y(0) = Tin,
            //            rozwiązanie y(x) = e^(-Ap*x)*(T*(e^(Ap*x)-1)+Tin),
            //            wężownica ma długość 1, temp. zbiornika na całej długośći jest taka sama,
            //            ilość energi przekazanej jest proporcjonalne do całki z y(x) od 0 do 1.
            fptype expMAp = exp(-Ap);
            Tout = T - expMAp * (T - Tin);
            T += time * K * (Tin - Tout) / Ap;
        } else {
            Tout = T;
        }
        T -= time * Twdelta;
        if (T < Twmin) {
            T = Twmin;
        }
    }

};



#endif
