#ifndef _WASM_HPP_
#define _WASM_HPP_


#define WASM_EXPORT(name) \
    __attribute__((used)) \
    __attribute__((export_name(#name)))


#define WASM_IMPORT(name) \
    __attribute__((used)) \
    __attribute__((import_name(#name)))



#endif // _WASM_HPP_
