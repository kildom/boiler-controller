#ifndef __ModelZasobnika_HPP
#define __ModelZasobnika_HPP

#include <math.h>


struct ModelZasobnika {

    double& T;       // out   Temperatura wody w zasobniku
    double& Tin;     // in    Temperatura wejściowa
    double& P;       // in    Przepływ wody w wężownicy
    double& Tout;    // out   Temperatura wyjściowa
    double& A;       // param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane)
    double& K;       // param Jak szybko wężownica przekazuje energię do zasobnika (jednostki nieznane)
    double& Twdelta; // param Jak szybko upływa temperatura ze zbiornika [°C/s].
    double& Twmin;   // param Minimalna temeratura zasobnika

    ModelZasobnika(double& T, double& Tin, double& P, double& Tout, double& A, double& K, double& Twdelta, double& Twmin):
        T(T), Tin(Tin), P(P), Tout(Tout), A(A), K(K), Twdelta(Twdelta), Twmin(Twmin) { }

    void step(double time)
    {
        if (P > 0.0) {
            double Ap = A / P;
            // założenie: rozkład temp. na wężownicy opisuje równanie y'(x) = (T-y) * Ap, y(0) = Tin,
            //            rozwiązanie y(x) = e^(-Ap*x)*(T*(e^(Ap*x)-1)+Tin),
            //            wężownica ma długość 1, temp. zbiornika na całej długośći jest taka sama,
            //            ilość energi przekazanej jest proporcjonalne do całki z y(x) od 0 do 1.
            double expMAp = exp(-Ap);
            Tout = T - expMAp * (T - Tin);
            T += time * K * (Tin - Tout) / Ap;
            T -= time * Twdelta;
            if (T < Twmin) {
                T = Twmin;
            }
        } else {
            Tout = T;
        }
    }

};



#endif
