#ifndef __ModelZaworu_HPP
#define __ModelZaworu_HPP


struct ModelZaworu {

    double& Z;        // out   Otwarcie zaworu 0..1
    double& Zdir;     // out   Aktualny kierunek otwierania zaworu -1, 0, +1
    bool&   Ron;      // in    Wejście przekaźnika załączającego 0, 1
    bool&   Rplus;    // in    Wejście przekaźnika zmieniającego kierunek 0 - zamykanie, 1 - otwieranie
    double& OpenTime; // param Czas otwarcia

    ModelZaworu(double& Z, double& Zdir, bool& Ron, bool& Rplus, double& OpenTime): Z(Z), Zdir(Zdir), Ron(Ron), Rplus(Rplus), OpenTime(OpenTime) { }

    void step(double time) {
        if (Ron > 0.0) {
            Zdir = Rplus > 0.0 ? +1.0 : -1.0;
            Z += Zdir * time * OpenTime;
            if (Z < 0.0) Z = 0.0;
            if (Z > 1.0) Z = 1.0;
        } else {
            Zdir = 0.0;
        }
    }
};


#endif
