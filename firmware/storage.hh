#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "global.hh"
#include "zawor.hh"

struct Storage {
    uint32_t magic1;
    uint32_t ver;
    Zawor::Storage zaw_powrotu;
    Zawor::Storage zaw_podl1;
    Zawor::Storage zaw_podl2;
    uint32_t crc;
    uint32_t magic2;
    static void init();
    static void write();
    static void update();
    static Storage storage;
private:
    static uint32_t getCrc();
};

class StorageAccessor {
public:
    Storage* operator*() const {
        Storage::init();
        return &Storage::storage;
    }
    Storage* operator->() const {
        Storage::init();
        return &Storage::storage;
    }
    operator Storage&() const {
        Storage::init();
        return Storage::storage;
    }
};

static const StorageAccessor storage;

#endif
