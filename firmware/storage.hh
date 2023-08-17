#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "global.hh"
#include "zawor.hh"
#include "relays.hh"
#include "temp.hh"

struct Storage {
    uint32_t magic1;
    uint32_t ver;
    Zawor::Storage zaw_powrotu;
    Zawor::Storage zaw_podl1;
    Zawor::Storage zaw_podl2;
    Relay::Storage relay;
    Temp::Storage temp;
    bool pelletDom;
    bool pelletCwu;
    bool elekDom;
    bool elekCwu;
    bool elekBezposrPodl;
    bool podl2;
    int elekStartupTime;
    int cwuTempMin;
    int cwuTempMax;
    int cwuTempCritical;
    int roomMinHeatTimePellet;
    int roomMinHeatTimeElek;
    uint32_t crc;
    uint32_t magic2;
    uint32_t _persistent_end;
    // ---------------- Non-permanent data
    uint64_t time;
    uint64_t roomHeatEnd;
    bool cwuHeatState;
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
