#ifndef MENU_HH_
#define MENU_HH_

#include "global.hh"

class Menu {
public:
    static void write(uint8_t data);
};

const char* tempToStr(char* dest, int temp); // TODO: different header file

#endif /* MENU_HH_ */
