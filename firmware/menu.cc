
#include <limits>

#include "global.hh"
#include "utils.hh"
#include "menu.hh"
#include "diag.hh"
#include "relays.hh"
#include "storage.hh"
#include "inputs.hh"

#include "autogen.inc"

static const int MAX_LINE_LEN = 128;

static char line[MAX_LINE_LEN];
static int lineIndex = 0;

struct MenuItem;

struct MenuItemCallbacks {
    bool (*action)(const MenuItem* item, char data);
    const char* (*text)(const MenuItem* item);
};

struct MenuItem {
    const char* name;
    const MenuItemCallbacks* callbacks;
    const void* sub;
};

static void showMenu();
static void showPath();
static bool menuFunc(const MenuItem* item, char data);


// menu item handler

static const MenuItemCallbacks menuCbk = { menuFunc };


// null item handler

static bool nullFunc(const MenuItem* item, char data)
{
    return false;
}

static const MenuItemCallbacks nullCbk = { nullFunc };


// generic bool item handler

struct BoolCallbacks {
    MenuItemCallbacks base;
    bool (*read)(const MenuItem* item);
    void (*write)(const MenuItem* item, bool value);
};

static bool boolGenericFunc(const MenuItem* item, char data)
{
    auto callbacks = containerOf(item->callbacks, BoolCallbacks::base);
    callbacks->write(item, !callbacks->read(item));
    return false;
}

static const char* boolGenericText(const MenuItem* item)
{
    auto callbacks = containerOf(item->callbacks, BoolCallbacks::base);
    return callbacks->read(item) ? ": 1" : ": 0";
}


// bool item handler

static bool boolRead(const MenuItem* item)
{
    return *(bool*)item->sub;
}

static void boolWrite(const MenuItem* item, bool value)
{
    *(bool*)item->sub = value;
}

static const BoolCallbacks boolCbk = {
    { boolGenericFunc, boolGenericText, }, boolRead, boolWrite,
};


// generic int item handler

template<typename T>
struct IntCallbacks {
    enum Kind {
        INT,
        TEMP,
        TIME,
    };
    MenuItemCallbacks base;
    T (*read)(const MenuItem* item);
    bool (*write)(const MenuItem* item, T value);
    Kind kind;
};

const char* tempToStr(char* dest, int temp)
{
    if (temp == Temp::INVALID) {
        strcpy(dest, "BŁĄD");
        return dest + strlen("BŁĄD");
    } else {
        return dest + sprintf(dest, "%d.%02d", temp / 100, abs(temp) % 100);
    }
}

static int tempFromStr(const char* text)
{
    char* endptr;
    auto i = strtol(text, &endptr, 10);
    auto f = 0;
    if (endptr[0] == '.' || endptr[0] == ',') {
        char* endptr2;
        f = strtol(&endptr[1], &endptr2, 10);
        int digits = endptr2 - endptr - 1;
        while (digits < 2) {
            f *= 10;
            digits++;
        }
        while (digits > 2) {
            f = (f + 5) / 10;
            digits--;
        }
    }
    if (i < 0) f = -f;
    return i * 100 + f;
}

static const char* timeToStr(char* dest, int t)
{
    int n;
    auto tt = t / 1000;
    auto ms = t % 1000;
    if (tt < 60 * 60) {
        auto sec = tt % 60;
        tt /= 60;
        n = sprintf(dest, "%d:%02d", tt, sec);
    } else {
        auto sec = tt % 60;
        tt /= 60;
        auto min = tt % 60;
        tt /= 60;
        n = sprintf(dest, "%d:%02d:%02d", tt, min, sec);
    }
    if (ms != 0) {
        n += sprintf(&dest[n], ".%03d", ms);
    }
    return dest + n;
}

static int timeFromStr(const char* text)
{
    char* ptr = (char*)text;
    int time = 0;
    do {
        auto i = strtol(ptr, &ptr, 10);
        time *= 60;
        time += i;
        if (*ptr == ':') {
            ptr++;
            continue;
        } else if (*ptr == '.') {
            ptr++;
            break;
        } else {
            return time * 1000;
        }
    } while (true);
    char* endptr;
    auto ms = strtol(ptr, &endptr, 10);
    int digits = endptr - ptr;
    while (digits < 3) {
        ms *= 10;
        digits++;
    }
    while (digits > 3) {
        ms = (ms + 5) / 10;
        digits--;
    }
    return time * 1000 + ms;
}


template<typename T>
static bool intGenericFunc(const MenuItem* item, char data)
{
    IntCallbacks<T>* callbacks = containerOf(item->callbacks, IntCallbacks<T>::base);

    if (data == '\r' || data == '\n') {
        if (lineIndex == 0) return false;
        line[lineIndex] = 0;
        int value;
        switch (callbacks->kind)
        {
            case IntCallbacks<T>::TEMP: value = tempFromStr(line); break;
            case IntCallbacks<T>::TIME: value = timeFromStr(line); break;
            default: value = atoi(line); break;
        }
        bool ok = value <= (int)std::numeric_limits<T>::max() && value >= (int)std::numeric_limits<T>::min();
        ok = ok && callbacks->write(item, (T)value);
        if (ok) return false;
        Diag::write("\r\nNieprawidłowa wartość!", -1, Diag::MENU);
        data = 0;
        // Fall back to the beginning
    }

    if (data == 0) {
        showPath();
        switch (callbacks->kind)
        {
            case IntCallbacks<T>::TEMP: tempToStr(line, (int)callbacks->read(item)); break;
            case IntCallbacks<T>::TIME: timeToStr(line, (int)callbacks->read(item)); break;
            default: sprintf(line, "%d", (int)callbacks->read(item)); break;
        }
        lineIndex = strlen(line);
        Diag::write(line, lineIndex, Diag::MENU);
        return true;
    } else if (lineIndex < MAX_LINE_LEN - 1 && (
            (data >= '0' && data <= '9') ||
            (data == '-' && lineIndex == 0 && callbacks->kind != IntCallbacks<T>::TIME) ||
            (data == ':' && callbacks->kind == IntCallbacks<T>::TIME) ||
            (data == '.' && callbacks->kind != IntCallbacks<T>::INT)
            )) {
        line[lineIndex++] = data;
        Diag::write(data, Diag::MENU);
        return true;
    } else if (((data == 8) || (data == 127)) && lineIndex > 0) {
        Diag::write("\x08 \x08", 3, Diag::MENU);
        lineIndex--;
        return true;
    } else {
        return true;
    }
}

template<typename T>
static const char* intGenericText(const MenuItem* item)
{
    auto callbacks = containerOf(item->callbacks, IntCallbacks<T>::base);
    strcpy(line, ": ");
    switch (callbacks->kind)
    {
        case IntCallbacks<T>::TEMP: tempToStr(&line[2], (int)callbacks->read(item)); break;
        case IntCallbacks<T>::TIME: timeToStr(&line[2], (int)callbacks->read(item)); break;
        default: sprintf(&line[2], "%d", (int)callbacks->read(item)); break;
    }
    return line;
}


// int item handler

template<typename T>
struct IntItemInfo {
    T* ptr;
    T max;
    T min;
};

template<typename T>
static T intRead(const MenuItem* item)
{
    return *((IntItemInfo<T>*)item->sub)->ptr;
}

template<typename T>
static bool intWrite(const MenuItem* item, T value)
{
    auto& info = *(IntItemInfo<T>*)item->sub;
    if (value < info.min || value > info.max) return false;
    *info.ptr = value;
    return true;
}

template<typename T>
static const IntCallbacks<T> intCbk = {
    { intGenericFunc<T>, intGenericText<T> }, intRead<T>, intWrite<T>, IntCallbacks<T>::INT,
};

template<typename T>
static const IntCallbacks<T> tempCbk = {
    { intGenericFunc<T>, intGenericText<T> }, intRead<T>, intWrite<T>, IntCallbacks<T>::TEMP,
};

static const IntCallbacks<int> timeCbk = {
    { intGenericFunc<int>, intGenericText<int> }, intRead<int>, intWrite<int>, IntCallbacks<int>::TIME,
};


// analog input item handler

static const char* tempInputText(const MenuItem* item)
{
    auto i = (Temp::Index)(int)item->sub;
    strcpy(line, ": ");
    tempToStr(&line[2], Temp::get(i));
    return line;
}

static const MenuItemCallbacks tempInputCbk = {
    nullFunc, tempInputText,
};


// relay toggle item handler

static bool relayToggleRead(const MenuItem* item)
{
    auto i = (Relay::Index)(int)item->sub;
    return Relay::get(i);
}

static void relayToggleWrite(const MenuItem* item, bool value)
{
    auto i = (Relay::Index)(int)item->sub;
    Relay::set(i, value);
}

static const BoolCallbacks relayToggleCbk = {
    { boolGenericFunc, boolGenericText, }, relayToggleRead, relayToggleWrite,
};


// relay invert item handler

static bool relayInvertRead(const MenuItem* item)
{
    auto i = (int)item->sub;
    return storage.relay.invert & (1 << i) ? true : false;
}

static void relayInvertWrite(const MenuItem* item, bool value)
{
    auto i = (int)item->sub;
    if (value) {
        storage.relay.invert |= (1 << i);
    } else {
        storage.relay.invert &= ~(1 << i);
    }
}

static const BoolCallbacks relayInvertCbk = {
    { boolGenericFunc, boolGenericText, }, relayInvertRead, relayInvertWrite,
};


// specialized item handlers

static bool saveStorageFunc(const MenuItem* item, char data)
{
    Storage::write();
    return false;
}


// The menu

static const MenuItem rootList[] = {
    STORAGE_MENU
    { "Zapisz do pamięci", (const MenuItemCallbacks[]){{ saveStorageFunc }}},
    { "Praca ręczna", &menuCbk, (const MenuItem[]){
        { "Przełącz wyjście", &menuCbk, (const MenuItem[]){
            { "paliwo", &relayToggleCbk.base, (void*)0 },
            { "piec", &relayToggleCbk.base, (void*)1 },
            { "elek", &relayToggleCbk.base, (void*)2 },
            { "pompa_powr", &relayToggleCbk.base, (void*)3 },
            { "zaw_powr", &relayToggleCbk.base, (void*)4 },
            { "zaw_powr_plus", &relayToggleCbk.base, (void*)5 },
            { "zaw_podl1", &relayToggleCbk.base, (void*)6 },
            { "zaw_podl1_plus", &relayToggleCbk.base, (void*)7 },
            { "pompa_podl1", &relayToggleCbk.base, (void*)8 },
            { "zaw_podl2", &relayToggleCbk.base, (void*)9 },
            { "zaw_podl2_plus", &relayToggleCbk.base, (void*)10 },
            { "pompa_podl2", &relayToggleCbk.base, (void*)11 },
            { "pompa_cwu", &relayToggleCbk.base, (void*)12 },
            { "buzzer", &relayToggleCbk.base, (void*)13 },
            {}}},
        { "Podgląd wejścia", &menuCbk, (const MenuItem[]){
            { "piec_pelet", &tempInputCbk, (void*)0 },
            { "piec_powrot", &tempInputCbk, (void*)1 },
            { "piec_elek", &tempInputCbk, (void*)2 },
            { "podl1", &tempInputCbk, (void*)3 },
            { "podl2", &tempInputCbk, (void*)4 },
            { "cwu", &tempInputCbk, (void*)5 },
            {}}},
        {}}},
    { "Wejścia/Wyjścia", &menuCbk, (const MenuItem[]){
        { "Przypisz wyjścia", &menuCbk, (const MenuItem[]){
            { "paliwo", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[0], 16 }}},
            { "piec", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[1], 16 }}},
            { "elek", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[2], 16 }}},
            { "pompa_powr", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[3], 16 }}},
            { "zaw_powr", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[4], 16 }}},
            { "zaw_powr_plus", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[5], 16 }}},
            { "zaw_podl1", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[6], 16 }}},
            { "zaw_podl1_plus", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[7], 16 }}},
            { "pompa_podl1", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[8], 16 }}},
            { "zaw_podl2", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[9], 16 }}},
            { "zaw_podl2_plus", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[10], 16 }}},
            { "pompa_podl2", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[11], 16 }}},
            { "pompa_cwu", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[12], 16 }}},
            { "buzzer", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.relay.map[13], 16 }}},
            {}}},
        { "Odwróć wyjścia", &menuCbk, (const MenuItem[]){
            { "paliwo", &relayInvertCbk.base, (void*)0 },
            { "piec", &relayInvertCbk.base, (void*)1 },
            { "elek", &relayInvertCbk.base, (void*)2 },
            { "pompa_powr", &relayInvertCbk.base, (void*)3 },
            { "zaw_powr", &relayInvertCbk.base, (void*)4 },
            { "zaw_powr_plus", &relayInvertCbk.base, (void*)5 },
            { "zaw_podl1", &relayInvertCbk.base, (void*)6 },
            { "zaw_podl1_plus", &relayInvertCbk.base, (void*)7 },
            { "pompa_podl1", &relayInvertCbk.base, (void*)8 },
            { "zaw_podl2", &relayInvertCbk.base, (void*)9 },
            { "zaw_podl2_plus", &relayInvertCbk.base, (void*)10 },
            { "pompa_podl2", &relayInvertCbk.base, (void*)11 },
            { "pompa_cwu", &relayInvertCbk.base, (void*)12 },
            { "brzeczyk", &relayInvertCbk.base, (void*)13 },
            {}}},
        { "Przypisz wejścia", &menuCbk, (const MenuItem[]){
            { "piec_pelet", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[0], 8 }}},
            { "piec_powrot", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[1], 8 }}},
            { "piec_elek", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[2], 8 }}},
            { "podl1", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[3], 8 }}},
            { "podl2", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[4], 8 }}},
            { "cwu", &intCbk<uint8_t>.base, (const IntItemInfo<uint8_t>[]) {{ &storage.temp.map[5], 8 }}},
            {}}},
        {}}},
    {}
};

static const MenuItem root = { "root", &menuCbk, rootList };

static const MenuItem* menuStack[20] = { &root };
static int menuStackLast = 0;

static char prevChar = 0;

static void showPath()
{
    const MenuItem *sub;
    Diag::write("\r\n > ", 5, Diag::MENU);
    for (int i = 1; i <= menuStackLast; i++) {
        sub = menuStack[i];
        Diag::write(sub->name, -1, Diag::MENU);
        if (sub->callbacks->text) {
            Diag::write(sub->callbacks->text(sub), -1, Diag::MENU);
        }
        Diag::write(" > ", 3, Diag::MENU);
    }
}

static void showMenu()
{
    const MenuItem *sub;
    if (menuStack[menuStackLast]->callbacks->action != menuFunc) return;
    sub = (const MenuItem *)menuStack[menuStackLast]->sub;
    char index = '1';
    Diag::write("\r\n------------------------------------------------------\r\n\r\n  ", -1, Diag::MENU);
    while (sub->name) {
        Diag::write(index, Diag::MENU);
        Diag::write(". ", -1, Diag::MENU);
        Diag::write(sub->name, -1, Diag::MENU);
        if (sub->callbacks->text) {
            Diag::write(sub->callbacks->text(sub), -1, Diag::MENU);
        }
        if (sub->callbacks->action == menuFunc) {
            Diag::write(" >>\r\n  ", -1, Diag::MENU);
        } else {
            Diag::write("\r\n  ", -1, Diag::MENU);
        }
        sub++;
        index++;
        if (index == ('9' + 1)) {
            index = 'A';
        }
    }
    showPath();
}

static const int KEY_NONE = -1;
static const int KEY_ENTER = -2;
static const int KEY_BACKSPACE = -3;

static int keyInput(char data)
{
    if (data == 0 || data == '\r' || (data == '\n' && prevChar != '\r')) {
        return KEY_ENTER;
    } else if ((data == 8) || (data == 127)) {
        return KEY_BACKSPACE;
    } else if (data >= '1' && data <= '9') {
        return data - '1';
    } else if (data >= 'A' && data <= 'Z') {
        return data - 'A' + 9;
    } else if (data >= 'a' && data <= 'z') {
        return data - 'a' + 9;
    } else {
        return KEY_NONE;
    }
}

static bool selectMenu(const MenuItem* item, int selected, char echoChar)
{
    int index = 0;
    const MenuItem* sub = (const MenuItem*)item->sub;
    while (sub->name) {
        if (index == selected) {
            menuStack[++menuStackLast] = sub;
            return sub->callbacks->action(sub, 0);
        }
        sub++;
        index++;
    }
    showPath();
    return true;
}

static bool menuFunc(const MenuItem* item, char data)
{
    int key = keyInput(data);
    if (key == KEY_ENTER) {
        showMenu();
        return true;
    } else if (key == KEY_BACKSPACE) {
        return false;
    } else if (key == KEY_NONE) {
        return true;
    } else {
        Diag::write(data, Diag::MENU);
        return selectMenu(item, key, data);
    }
}


void Menu::write(uint8_t data)
{
    const MenuItem* top = menuStack[menuStackLast];
    bool stay = top->callbacks->action(top, data);
    if (!stay && menuStackLast > 0) {
        menuStackLast--;
        menuStack[menuStackLast]->callbacks->action(menuStack[menuStackLast], 0);
    }
    prevChar = data;
}

