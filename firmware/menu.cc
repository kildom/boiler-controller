
#include "menu.hh"
#include "diag.hh"
#include "relays.hh"
#include "storage.hh"

static const int MAX_LINE_LEN = 128;

static char line[MAX_LINE_LEN];
static int lineIndex = 0;

struct MenuItem {
    const char* name;
    bool (*func)(const MenuItem* item, char data);
    const void* sub;
    const char* (*nameFunc)(const MenuItem* item);
};

static void showMenu();
static void showPath();
static bool menuFunc(const MenuItem* item, char data);

static bool nullFunc(const MenuItem* item, char data)
{
    return false;
}

static bool boolFunc(const MenuItem* item, char data)
{
    bool* value = (bool*)item->sub;
    *value = !*value;
    return false;
}

static const char* boolText(const MenuItem* item)
{
    bool* value = (bool*)item->sub;
    return *value ? ": 1" : ": 0";
}


static bool inputIntFunc(const MenuItem* item, char data, int& value)
{
    if (data == 0) {
        sprintf(line, "%d", value);
        lineIndex = strlen(line);
        showPath();
        Diag::write(line, lineIndex, Diag::MENU);
        return true;
    } else if (data == '\r' || data == '\n') {
        if (lineIndex == 0) return false;
        line[lineIndex] = 0;
        value = atoi(line);
        return false;
    } else if (data >= 32 && data < 127 && lineIndex < MAX_LINE_LEN - 1) {
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

static bool relayToggleFunc(const MenuItem* item, char data)
{
    auto i = (Relay::Index)(int)item->sub;
    Relay::set(i, !Relay::get(i));
    return false;
}

static const char* relayValueText(const MenuItem* item)
{
    auto i = (Relay::Index)(int)item->sub;
    return Relay::get(i) ? "1" : "0";
}

static bool relayAssignFunc(const MenuItem* item, char data)
{
    static int value;
    auto i = (int)item->sub;
    if (data == 0) value = storage->relay.map[i];
    bool active = inputIntFunc(item, data, value);
    if (!active && value >= 0 && value < 16) storage->relay.map[i] = value;
    return active;
}

static const char* relayAssignText(const MenuItem* item)
{
    auto i = (int)item->sub;
    sprintf(line, "%d", storage->relay.map[i]);
    return line;
}

static bool relayInvertFunc(const MenuItem* item, char data)
{
    auto i = (int)item->sub;
    int bit = (1 << i);
    storage->relay.invert ^= bit;
    return false;
}

static const char* relayInvertText(const MenuItem* item)
{
    auto i = (int)item->sub;
    return (storage->relay.invert & (1 << i)) ? "ODWR." : "-";
}

static const char* analogValueText(const MenuItem* item)
{
    auto i = (Temp::Index)(int)item->sub;
    auto x = Temp::get(i);
    if (x == Temp::INVALID) {
        return "BŁĄD!!!";
    } else {
        sprintf(line, "%d.%02d", x / 100, abs(x) % 100);
    }
    return line;
}


static bool analogAssignFunc(const MenuItem* item, char data)
{
    static int value;
    auto i = (int)item->sub;
    if (data == 0) value = storage->temp.map[i];
    bool active = inputIntFunc(item, data, value);
    if (!active && value >= 0 && value < 9) storage->temp.map[i] = value; // TODO: define with analog inputs count
    return active;
}

static const char* analogAssignText(const MenuItem* item)
{
    auto i = (int)item->sub;
    sprintf(line, "%d", storage->temp.map[i]);
    return line;
}

static bool saveStorageFunc(const MenuItem* item, char data)
{
    Storage::write();
    return false;
}

static bool tempFunc(const MenuItem* item, char data)
{
    int* value = (int*)item->sub;
    if (data == 0) {
        sprintf(line, "%d.%02d", *value / 100, abs(*value) % 100);
        lineIndex = strlen(line);
        showPath();
        Diag::write(line, lineIndex, Diag::MENU);
        return true;
    } else if (data == '\r' || data == '\n') {
        if (lineIndex == 0) return false;
        line[lineIndex] = 0;
        value = atoi(line);
        return false;
    } else if (data >= 32 && data < 127 && lineIndex < MAX_LINE_LEN - 1) {
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
    bool active = inputIntFunc(item, data, *value);
    if (!active && *value >= 0 && *value < 9) storage->temp.map[i] = value; // TODO: define with analog inputs count
    return active;
}

static const char* tempText(const MenuItem* item)
{
    int* value = (int*)item->sub;
    sprintf(line, "%d.%02d", *value / 100, abs(*value) % 100);
    return line;
}

static const MenuItem rootList[] = {
    { "Tryby", menuFunc, (MenuItem[]){
        { "Pellet C.O.", boolFunc, &Storage::storage.pelletDom, boolText },
        { "Pellet C.W.U.", boolFunc, &Storage::storage.pelletCwu, boolText },
        { "Elekt. C.O.", boolFunc, &Storage::storage.elekDom, boolText },
        { "Elekt. C.W.U.", boolFunc, &Storage::storage.elekCwu, boolText },
        { "Elekt. bez zaw. podl.", boolFunc, &Storage::storage.elekBezposrPodl, boolText },
        {}}},
    { "Praca ręczna", menuFunc, (MenuItem[]){
        { "Przełącz wyjście", menuFunc, (MenuItem[]){
            { "paliwo: ", relayToggleFunc, (void*)0, relayValueText },
            { "piec: ", relayToggleFunc, (void*)1, relayValueText },
            { "elek: ", relayToggleFunc, (void*)2, relayValueText },
            { "pompa_powr: ", relayToggleFunc, (void*)3, relayValueText },
            { "zaw_powr: ", relayToggleFunc, (void*)4, relayValueText },
            { "zaw_powr_plus: ", relayToggleFunc, (void*)5, relayValueText },
            { "zaw_podl1: ", relayToggleFunc, (void*)6, relayValueText },
            { "zaw_podl1_plus: ", relayToggleFunc, (void*)7, relayValueText },
            { "pompa_podl1: ", relayToggleFunc, (void*)8, relayValueText },
            { "zaw_podl2: ", relayToggleFunc, (void*)9, relayValueText },
            { "zaw_podl2_plus: ", relayToggleFunc, (void*)10, relayValueText },
            { "pompa_podl2: ", relayToggleFunc, (void*)11, relayValueText },
            { "pompa_cwu: ", relayToggleFunc, (void*)12, relayValueText },
            { "buzzer: ", relayToggleFunc, (void*)13, relayValueText },
            {}}},
        { "Podgląd wejścia", menuFunc, (MenuItem[]){
            { "piec_pelet: ", nullFunc, (void*)0, analogValueText },
            { "piec_powrot: ", nullFunc, (void*)1, analogValueText },
            { "piec_elek: ", nullFunc, (void*)2, analogValueText },
            { "podl1: ", nullFunc, (void*)3, analogValueText },
            { "podl2: ", nullFunc, (void*)4, analogValueText },
            { "cwu: ", nullFunc, (void*)5, analogValueText },
            {}}},
        {}}},
    { "Konfiguracja", menuFunc, (MenuItem[]){
        { "Przypisz wyjścia", menuFunc, (MenuItem[]){
            { "paliwo: ", relayAssignFunc, (void*)0, relayAssignText },
            { "piec: ", relayAssignFunc, (void*)1, relayAssignText },
            { "elek: ", relayAssignFunc, (void*)2, relayAssignText },
            { "pompa_powr: ", relayAssignFunc, (void*)3, relayAssignText },
            { "zaw_powr: ", relayAssignFunc, (void*)4, relayAssignText },
            { "zaw_powr_plus: ", relayAssignFunc, (void*)5, relayAssignText },
            { "zaw_podl1: ", relayAssignFunc, (void*)6, relayAssignText },
            { "zaw_podl1_plus: ", relayAssignFunc, (void*)7, relayAssignText },
            { "pompa_podl1: ", relayAssignFunc, (void*)8, relayAssignText },
            { "zaw_podl2: ", relayAssignFunc, (void*)9, relayAssignText },
            { "zaw_podl2_plus: ", relayAssignFunc, (void*)10, relayAssignText },
            { "pompa_podl2: ", relayAssignFunc, (void*)11, relayAssignText },
            { "pompa_cwu: ", relayAssignFunc, (void*)12, relayAssignText },
            { "buzzer: ", relayAssignFunc, (void*)13, relayAssignText },
            {}}},
        { "Odwróć wyjścia", menuFunc, (MenuItem[]){
            { "paliwo: ", relayInvertFunc, (void*)0, relayInvertText },
            { "piec: ", relayInvertFunc, (void*)1, relayInvertText },
            { "elek: ", relayInvertFunc, (void*)2, relayInvertText },
            { "pompa_powr: ", relayInvertFunc, (void*)3, relayInvertText },
            { "zaw_powr: ", relayInvertFunc, (void*)4, relayInvertText },
            { "zaw_powr_plus: ", relayInvertFunc, (void*)5, relayInvertText },
            { "zaw_podl1: ", relayInvertFunc, (void*)6, relayInvertText },
            { "zaw_podl1_plus: ", relayInvertFunc, (void*)7, relayInvertText },
            { "pompa_podl1: ", relayInvertFunc, (void*)8, relayInvertText },
            { "zaw_podl2: ", relayInvertFunc, (void*)9, relayInvertText },
            { "zaw_podl2_plus: ", relayInvertFunc, (void*)10, relayInvertText },
            { "pompa_podl2: ", relayInvertFunc, (void*)11, relayInvertText },
            { "pompa_cwu: ", relayInvertFunc, (void*)12, relayInvertText },
            { "brzeczyk: ", relayInvertFunc, (void*)13, relayInvertText },
            {}}},
        { "Przypisz wejścia", menuFunc, (MenuItem[]){
            { "piec_pelet: ", analogAssignFunc, (void*)0, analogAssignText },
            { "piec_powrot: ", analogAssignFunc, (void*)1, analogAssignText },
            { "piec_elek: ", analogAssignFunc, (void*)2, analogAssignText },
            { "podl1: ", analogAssignFunc, (void*)3, analogAssignText },
            { "podl2: ", analogAssignFunc, (void*)4, analogAssignText },
            { "cwu: ", analogAssignFunc, (void*)5, analogAssignText },
            {}}},
        { "Temperatury", menuFunc, (MenuItem[]){
            { "cwuTempMin", tempFunc, &Storage::storage.cwuTempMin, tempText },
            { "cwuTempMax", tempFunc, &Storage::storage.cwuTempMax, tempText },
            { "cwuTempCritical", tempFunc, &Storage::storage.cwuTempCritical, tempText },
            {}}},
        { "Załącz drugą podl.", boolFunc, &Storage::storage.podl2, boolText },
        {}}},
    { "Zapisz do pamięci", saveStorageFunc },
    {}
};

static const MenuItem root = { "root", menuFunc, rootList };

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
        if (sub->nameFunc) {
            Diag::write(sub->nameFunc(sub), -1, Diag::MENU);
        }
        Diag::write(" > ", 3, Diag::MENU);
    }
}

static void showMenu()
{
    const MenuItem *sub;
    if (menuStack[menuStackLast]->func != menuFunc) return;
    sub = (const MenuItem *)menuStack[menuStackLast]->sub;
    char index = '1';
    Diag::write("\r\n------------------------------------------------------\r\n\r\n  ", -1, Diag::MENU);
    while (sub->name) {
        Diag::write(index, Diag::MENU);
        Diag::write(". ", -1, Diag::MENU);
        Diag::write(sub->name, -1, Diag::MENU);
        if (sub->nameFunc) {
            Diag::write(sub->nameFunc(sub), -1, Diag::MENU);
        }
        if (sub->func != menuFunc) {
            Diag::write("\r\n  ", -1, Diag::MENU);
        } else {
            Diag::write(" >>\r\n  ", -1, Diag::MENU);
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
            return sub->func(sub, 0);
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
    bool stay = top->func(top, data);
    if (!stay && menuStackLast > 0) {
        menuStackLast--;
        menuStack[menuStackLast]->func(menuStack[menuStackLast], 0);
    }
    prevChar = data;
}

