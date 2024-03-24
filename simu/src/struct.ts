
export interface ModelState {

    // Parametry symulacji
    /** param Prędkość symulacji */
    speed: number;
    /** param Maksymalny krok symulacji */
    maxStepSize: number;
    /** param Okres jednego odświerzenia wizualizacji */
    period: number;
    /** calc Simulation is running */
    running: boolean;

    // Ogólne
    /** calc Czas od początku symulacji */
    Time: number;
    /** param Czas otwarcia zaworu */
    OpenTime: number;

    // Kocioł na pellet
    /** calc Temperatura powrotu */
    Tpowr: number;
    /** mod Temperatura wyjściowa pieca */
    Tpiec: number;
    /** calc Aktualny przepływ pompki pieca */
    P0: number;
    /** param Przepływ pompki pieca, gdy pracuje */
    P0v: number;
    /** mod Zawór powrotu pieca (0 - niska temp. powrotu, 1 - wysoka temp. powrotu) */
    Z0: number;
    /** mod Aktualny kierunek zaworu powrotu pieca */
    Z0dir: number;

    // Kocioł elektryczny
    /** mod Temperatura wyjścia kotła elektrycznego */
    Tele: number;
    /** param Temperatura zadana kotła elektrycznego */
    Tzadele: number;
    /** calc Przepływ pompy kotła elektrycznego */
    P4: number;
    /** param Przepływ pompy kotła elektrycznego, gdy pracuje */
    P4v: number;
    /** mod Załączenie pompy przez kocioł elekt. */
    Rele: boolean;
    /** param Czas startu kotła elektrycznego */
    EleCzasStart: number;
    /** param Czas zatrzymania kotła elektrycznego */
    EleCzasStop: number;
    /** param Max. różnica temperatury przy przepływie "1" */
    EleMoc: number;

    // Sprzęgło hydrauliczne
    /** calc Temeratura zimnego wyjścia ze sprzęgła */
    Tspz: number;
    /** calc Temeratura ciepłego wyjścia ze sprzęgła */
    Tspc: number;
    /** calc Temeratura zimnego wejścia do sprzęgła */
    Twejspz: number;
    /** calc Temeratura ciepłego wejścia do sprzęgła */
    Twejspc: number;

    // Podłogówka 1
    /** calc Temperatura podłogówki 1 */
    Tpodl1: number;
    /** mod Temperatura wyjściowa podłogówki 1 */
    Twyj1: number;
    /** calc Przepływ pompy podłogówki 1 */
    P1: number;
    /** param Przepływ pompy podłogówki 1, gdy pracuje */
    P1v: number;
    /** mod Zawór podłogówki 1 (0 - niska. temp. podł, 1 - wysoka temp. podł) */
    Z1: number;
    /** mod Aktualny kierunek zaworu podłogówki 1 */
    Z1dir: number;
    /** mod Temperatura wylewki 1 */
    Twyl1: number;
    /** calc Jak szybko upływa temperatura z wylewki 1 [°C/s]. */
    Tdelta1: number;

    // Podłogówka 2
    /** calc Temperatura podłogówki 2 */
    Tpodl2: number;
    /** mod Temperatura wyjściowa podłogówki 2 */
    Twyj2: number;
    /** calc Przepływ pompy podłogówki 1 */
    P2: number;
    /** param Przepływ pompy podłogówki 2, gdy pracuje */
    P2v: number;
    /** mod Zawór podłogówki 2 (0 - niska. temp. podł, 1 - wysoka temp. podł) */
    Z2: number;
    /** mod Aktualny kierunek zaworu podłogówki 2 */
    Z2dir: number;
    /** mod Temperatura wylewki 2 */
    Twyl2: number;
    /** calc Jak szybko upływa temperatura z wylewki 2 [°C/s] */
    Tdelta2: number;

    // Podłogówka - wspólne
    /** param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane) */
    PodlA: number;
    /** param Jak szybko wężownica przekazuje energię do wylewki (jednostki nieznane) */
    PodlK: number;
    /** param Jak szybko upływa temperatura z wylewki na 1 °C różnicy [°C/s / °C] */
    Tpoddelta: number;

    // Dom
    /** calc Temperatura domu */
    Tdom: number;
    /** param Temperatura zadana domu */
    Tzaddom: number;
    /** param Temperatura na zwenątrz */
    Tzewn: number;
    /** param Jak szybko zmienia się temperatura domu na 1 °C różnicy z wylewką [°C/s / °C] */
    Tdomdelta: number;
    /** param Jak szybko zmienia się temperatura domu na 1 °C różnicy z zewnętrzem [°C/s / °C] */
    Tzewndelta: number;

    // CWU
    /** mod Wyjście zasobnika CWU */
    Twyj3: number;
    /** mod Temperatura wody CWU */
    Tzas: number;
    /** calc Przepływ pompy CWU */
    P3: number;
    /** param Przepływ pompy CWU, gdy pracuje */
    P3v: number;
    /** param Jak szybko spada temperatura po długości wężownicy przy przepływie P=1 (jednostki nieznane) */
    ZasA: number;
    /** param Jak szybko wężownica przekazuje energię do zasobnika (jednostki nieznane) */
    ZasK: number;
    /** param Jak szybko upływa temperatura ze zbiornika [°C/s]. */
    ZasTwdelta: number;
    /** param Minimalna temeratura zasobnika */
    ZasTwmin: number;

    // Przekaźniki
    /** out Relay 0 - odcięcie paliwa */
    R0: boolean;
    /** out Relay 1 - włączenie pieca */
    R1: boolean;
    /** out Relay 2 - włączenie kotła elek. */
    R2: boolean;
    /** out Relay 3 - pompa pieca */
    R3: boolean;
    /** out Relay 4 - zawór powrotu on/off */
    R4: boolean;
    /** out Relay 5 - zawór powrotu +/- */
    R5: boolean;
    /** out Relay 6 - zawór podl. 1 on/off */
    R6: boolean;
    /** out Relay 7 - zawór podl. 1 +/- */
    R7: boolean;
    /** out Relay 8 - pompa podłogówki 1 */
    R8: boolean;
    /** out Relay 9 - zawór podl. 2 on/off */
    R9: boolean;
    /** out Relay 10 - zawór podl. 2 +/- */
    R10: boolean;
    /** out Relay 11 - pompa podłogówki 2 */
    R11: boolean;
    /** out Relay 12 - pompa CWU */
    R12: boolean;
    /** out Relay 13 - buzzer */
    R13: boolean;

    // Wejścia
    /** in Input 0 - ster. pokojowy */
    IN0: boolean;
    /** in Input 1 - podajnik pelletu */
    IN1: boolean;

};
