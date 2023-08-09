#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdint.h>
#include "zawor.hh"

struct Storage {
    uint32_t ver;
    Zawor::Storage zaw_powrotu;
    Zawor::Storage zaw_podl1;
    Zawor::Storage zaw_podl2;
    static void init();
    static Storage storage;
};

class StorageAccessor {
public:
    Storage* operator*() const {
        if (Storage::storage.ver == 0) {
            Storage::init();
        }
        return &Storage::storage;
    }
    Storage* operator->() const {
        if (Storage::storage.ver == 0) {
            Storage::init();
        }
        return &Storage::storage;
    }
    operator Storage&() const {
        if (Storage::storage.ver == 0) {
            Storage::init();
        }
        return Storage::storage;
    }
};

static const StorageAccessor storage;

#endif
