
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "log.hh"
#include "crc.hh"
#include "lowlevel.hh"
#include "utils.hh"
#include "storage.hh"

#define PERSISTENT_SIZE offsetof(Storage, _persistent_end)
#define STORAGE_MAGIC1 (0x5B195F05 + (PERSISTENT_SIZE << 12))
#define STORAGE_MAGIC2 (0x708ADBC9 + (PERSISTENT_SIZE << 12))

#define SLOT0_DIRTY 1
#define SLOT1_DIRTY 2

static int slot_dirty_flags = 0;
static int write_slot = 0;
static uint32_t write_state;

uint8_t writeBuffer[PERSISTENT_SIZE]; // Used for async write
uint8_t snapBuffer[PERSISTENT_SIZE];  // Snapshot buffer

Storage Storage::storage = {
    .ver = 0,
};

static const Storage storageInit = {
    .magic1 = STORAGE_MAGIC1,
    .ver = 0,
    .zaw_powrotu = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .czas_przerwy = 10 * 1000,
        .czas_pracy_max = 4 * 1000,
        .czas_pracy_min = 2 * 1000,
        .korekta = 10,
        .critical = 8500,
    },
    .zaw_podl1 = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .czas_przerwy = 10 * 1000,
        .czas_pracy_max = 4 * 1000,
        .czas_pracy_min = 2 * 1000,
        .korekta = 10,
        .critical = 4500,
    },
    .zaw_podl2 = {
        .czas_otwarcia = 2 * 60 * 1000,
        .czas_min_otwarcia = 2 * 60 * 1000 / 100 * 3,
        .czas_przerwy = 10 * 1000,
        .czas_pracy_max = 4 * 1000,
        .czas_pracy_min = 2 * 1000,
        .korekta = 10,
        .critical = 4500,
    },
    .relay = {
        .map = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, },
        .invert = 0,
    },
    .temp = {
        .map = { 0, 1, 2, 3, 4, 5, },
    },
    .pelletDom = false,
    .pelletCwu = false,
    .elekDom = true, // TODO: set to false
    //.elekCwu = false,
    //.elekBezposrPodl = false,
    //.podl2 = true,
    .elekStartupTime = 4 * 60 * 1000,
    .elekCritical = 6500,
    .elekOffTime = 60 * 1000,
    .cwuTempMin = 3500,
    .cwuTempMax = 5000,
    .cwuTempCritical = 6500,
    .roomMinHeatTimePellet = 2 * 60 * 60 * 1000,
    .roomMinHeatTimeElek = 15 * 60 * 1000,
    .podlFaultDelay = 2 * 60 * 1000,
    .podlFaultPiecTemp = 6000,
    .crc = 0,
    .magic2 = STORAGE_MAGIC2,
    // ---------------- Non-permanent data
    .time = 0,
    .roomHeatEnd = 0,
    .cwuHeatState = false,
};

uint32_t Storage::getCrc()
{
    Crc32 crc;
    auto old = storage.crc;
    storage.crc = 0;
    crc.data((uint8_t*)&storage, PERSISTENT_SIZE);
    storage.crc = old;
    return crc.value;
}

void Storage::init()
{
    if (Storage::storage.magic1 == 0) {

        // Copy initial non-persistent values
        memcpy((uint8_t*)&storage + PERSISTENT_SIZE, (uint8_t*)&storageInit + PERSISTENT_SIZE, sizeof(storage) - PERSISTENT_SIZE);

        // Read and validate slot 0
        store_read(0, (uint8_t *)&storage, PERSISTENT_SIZE);
        uint32_t slot0_ver = storage.ver;
        if (storage.magic1 != STORAGE_MAGIC1 || storage.magic2 != STORAGE_MAGIC2 || getCrc() != storage.crc) {
            DBG("Slot 0 invalid");
            slot_dirty_flags |= SLOT0_DIRTY;
        }

        // Read and validate slot 1
        store_read(1, (uint8_t *)&storage, PERSISTENT_SIZE);
        uint32_t slot1_ver = storage.ver;
        if (storage.magic1 != STORAGE_MAGIC1 || storage.magic2 != STORAGE_MAGIC2 || getCrc() != storage.crc) {
            DBG("Slot 1 invalid");
            slot_dirty_flags |= SLOT1_DIRTY;
        }

        if (slot_dirty_flags == 0) { // If both are valid, compare its versions
            if ((int)(slot0_ver - slot1_ver) < 0) {
                DBG("Slot 0 out of date");
                slot_dirty_flags |= SLOT0_DIRTY;
            } else if ((int)(slot0_ver - slot1_ver) > 0) {
                DBG("Slot 1 out of date");
                slot_dirty_flags |= SLOT1_DIRTY;
            }
        }

        if (slot_dirty_flags == SLOT1_DIRTY) { // If slot 1 is invalid, and slot 0 valid, load slot 0 again
            DBG("Using Slot 0");
            store_read(0, (uint8_t *)&storage, PERSISTENT_SIZE);
        } else if (slot_dirty_flags == (SLOT0_DIRTY | SLOT1_DIRTY)) { // If both are invalid, load default
            ERR("Loading default settings");
            memcpy(&storage, &storageInit, PERSISTENT_SIZE);
            storage.crc = getCrc();
        } else {
            DBG("Using Slot 1");
        }

        // Copy data to snapshot buffer if we have any dirty slots
        memcpy(snapBuffer, &storage, PERSISTENT_SIZE);
    }
}


void Storage::update()
{
    // If there are no dirty slots and write is not in progress, do nothing
    if (slot_dirty_flags == 0 && write_state == 0) return;

    if (write_state == 0) { // We are not currently writing, but we have new dirty slots
        // Copy to avoid corruption
        memcpy(writeBuffer, snapBuffer, PERSISTENT_SIZE);
        // If current write_slot is not dirty, toggle the write_slot
        if (!(slot_dirty_flags & (1 << write_slot))) {
            write_slot ^= 1;
        }
        // Clear dirty flag, because we just starting write
        slot_dirty_flags &= ~(1 << write_slot);
        ERR("Writing to slot %d", write_slot);
    }

    store_write(&write_state, write_slot, writeBuffer, PERSISTENT_SIZE);
}


void Storage::write()
{
    storage.ver++;
    storage.crc = getCrc();
    memcpy(snapBuffer, &storage, PERSISTENT_SIZE);
}
