#include "global.hh"
#include "inputs.hh"
#include "storage.hh"

bool Input::get(Index index) {
    int indexLo = storage.input.map[index];
    bool res = input(indexLo);
    if (storage.input.invert & (1 << index)) {
        res = !res;
    }
    return res;
}

