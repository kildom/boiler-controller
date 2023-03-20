
#include "model.hpp"

#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))


State state;


WASM_EXPORT(getState)
State* getState() {
    return &state;
}


WASM_EXPORT(steps)
void steps(int count, double maxStepTime)
{
    for (int i = 0; i < count; i++) {
        state.step(maxStepTime);
    }
}

