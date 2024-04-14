
#include "global.hh"
#include "storage.hh"
#include "temp.hh"

static int rawToTemp(int x)
{
    //return (((312936 * x - 620439185) >> 11) * x - 46488827) >> 15; // KTY81/210 + 1.5K resistor on 12 bits ADC
    //return (((19559 * x - 620439186) >> 15) * x - 46488827) >> 15;  // KTY81/210 + 1.5K resistor on 16 bits ADC
    //return (((400989 * x + 99792223) >> 12) * x - 336445037) >> 15; // KTY81/210 + 2.21K resistor on 12 bits ADC
    return (((25062 * x + 99792227) >> 16) * x - 336445037) >> 15;    // KTY81/210 + 2.21K resistor on 16 bits ADC
}

int Temp::get(Index index)
{
    int indexLo = storage.temp.map[index];
    auto raw = analog_input(indexLo);
    auto temp = rawToTemp(raw);
    if (temp < -1000 || temp > 11000) {
        return INVALID;
    }
    return temp;
}
