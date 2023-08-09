
#include <stdint.h>
#include <string.h>
#include "log.hh"
#include "lowlevel.hh"
#include "storage.hh"

#define STORAGE_VERSION (0x5B095F00 + (sizeof(Storage) << 12))

Storage Storage::storage = {
    .ver = 0,
};

static const Storage storageInit = {
    .ver = STORAGE_VERSION,
    .zaw_powrotu = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .odw_kierunek = false,
    },
    .zaw_podl1 = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .odw_kierunek = false,
    },
    .zaw_podl2 = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .odw_kierunek = false,
    }
};

void Storage::init()
{
    store_read((uint8_t *)&storage, sizeof(storage));
    if (storage.ver != STORAGE_VERSION) {
        ERR("Storage version 0x%08X, expected 0x%08X.", storage.ver, (unsigned int)STORAGE_VERSION);
        DBG("%d", storage.zaw_podl1.czas_otwarcia);
        memcpy(&storage, &storageInit, sizeof(storage));
        DBG("%d", storage.zaw_podl1.czas_otwarcia);
        store_write((uint8_t *)&storage, sizeof(storage));
        DBG("%d", storage.zaw_podl1.czas_otwarcia);
    }
}
