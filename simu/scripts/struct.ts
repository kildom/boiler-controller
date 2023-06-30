
export interface State {

    // Parametry symulacji
    /** param Prędkość symulacji */
    speed: number;
    /** param Maksymalny krok symulacji */
    maxStepSize: number;
    /** param Okres jednego odświerzenia wizualizacji */
    period: number;

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

    // Wejścia
    /** in Input 0 - tryb zima */
    IN0: boolean;
    /** in Input 1 - wybrany kocioł elektryczny */
    IN1: boolean;
    /** in Input 2 - ster. pokojowy */
    IN2: boolean;

};

export function get(view: DataView): State {
    return {
        speed: view.getFloat64(0, true),
        maxStepSize: view.getFloat64(8, true),
        period: view.getFloat64(16, true),
        Time: view.getFloat64(24, true),
        OpenTime: view.getFloat64(32, true),
        Tpowr: view.getFloat64(40, true),
        Tpiec: view.getFloat64(48, true),
        P0: view.getFloat64(56, true),
        P0v: view.getFloat64(64, true),
        Z0: view.getFloat64(72, true),
        Z0dir: view.getFloat64(80, true),
        Tele: view.getFloat64(88, true),
        Tzadele: view.getFloat64(96, true),
        P4: view.getFloat64(104, true),
        P4v: view.getFloat64(112, true),
        Rele: view.getInt8(120) ? true : false,
        EleCzasStart: view.getFloat64(128, true),
        EleCzasStop: view.getFloat64(136, true),
        EleMoc: view.getFloat64(144, true),
        Tspz: view.getFloat64(152, true),
        Tspc: view.getFloat64(160, true),
        Twejspz: view.getFloat64(168, true),
        Twejspc: view.getFloat64(176, true),
        Tpodl1: view.getFloat64(184, true),
        Twyj1: view.getFloat64(192, true),
        P1: view.getFloat64(200, true),
        P1v: view.getFloat64(208, true),
        Z1: view.getFloat64(216, true),
        Z1dir: view.getFloat64(224, true),
        Twyl1: view.getFloat64(232, true),
        Tdelta1: view.getFloat64(240, true),
        Tpodl2: view.getFloat64(248, true),
        Twyj2: view.getFloat64(256, true),
        P2: view.getFloat64(264, true),
        P2v: view.getFloat64(272, true),
        Z2: view.getFloat64(280, true),
        Z2dir: view.getFloat64(288, true),
        Twyl2: view.getFloat64(296, true),
        Tdelta2: view.getFloat64(304, true),
        PodlA: view.getFloat64(312, true),
        PodlK: view.getFloat64(320, true),
        Tpoddelta: view.getFloat64(328, true),
        Tdom: view.getFloat64(336, true),
        Tzaddom: view.getFloat64(344, true),
        Tzewn: view.getFloat64(352, true),
        Tdomdelta: view.getFloat64(360, true),
        Tzewndelta: view.getFloat64(368, true),
        Twyj3: view.getFloat64(376, true),
        Tzas: view.getFloat64(384, true),
        P3: view.getFloat64(392, true),
        P3v: view.getFloat64(400, true),
        ZasA: view.getFloat64(408, true),
        ZasK: view.getFloat64(416, true),
        ZasTwdelta: view.getFloat64(424, true),
        ZasTwmin: view.getFloat64(432, true),
        R0: view.getInt8(440) ? true : false,
        R1: view.getInt8(441) ? true : false,
        R2: view.getInt8(442) ? true : false,
        R3: view.getInt8(443) ? true : false,
        R4: view.getInt8(444) ? true : false,
        R5: view.getInt8(445) ? true : false,
        R6: view.getInt8(446) ? true : false,
        R7: view.getInt8(447) ? true : false,
        R8: view.getInt8(448) ? true : false,
        R9: view.getInt8(449) ? true : false,
        R10: view.getInt8(450) ? true : false,
        R11: view.getInt8(451) ? true : false,
        R12: view.getInt8(452) ? true : false,
        IN0: view.getInt8(453) ? true : false,
        IN1: view.getInt8(454) ? true : false,
        IN2: view.getInt8(455) ? true : false,
    };
};

