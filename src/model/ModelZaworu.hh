#ifndef __ModelZaworu_HPP
#define __ModelZaworu_HPP

#include "modelCommon.hh"

struct ModelZaworu {

    fptype& Z;        // out   Otwarcie zaworu 0..1
    fptype& Zdir;     // out   Aktualny kierunek otwierania zaworu -1, 0, +1
    bool&   Ron;      // in    Wejście przekaźnika załączającego 0, 1
    bool&   Rplus;    // in    Wejście przekaźnika zmieniającego kierunek 0 - zamykanie, 1 - otwieranie
    fptype& OpenTime; // param Czas otwarcia

    ModelZaworu(fptype& Z, fptype& Zdir, bool& Ron, bool& Rplus, fptype& OpenTime): Z(Z), Zdir(Zdir), Ron(Ron), Rplus(Rplus), OpenTime(OpenTime) { }

    void step(fptype time) {
        if (Ron > 0.0_f) {
            Zdir = Rplus > 0.0_f ? +1.0_f : -1.0_f;
            Z += Zdir * time / OpenTime;
            if (Z < 0.0_f) Z = 0.0_f;
            if (Z > 1.0_f) Z = 1.0_f;
        } else {
            Zdir = 0.0_f;
        }
    }
};


#endif
