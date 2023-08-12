
#include <stdint.h>
#include <string.h>
#include "log.hh"
#include "crc.hh"
#include "lowlevel.h"
#include "storage.hh"

#define STORAGE_MAGIC1 (0x5B095F03 + (sizeof(Storage) << 12))
#define STORAGE_MAGIC2 (0x708ADBC9 + (sizeof(Storage) << 12))

#define SLOT0_DIRTY 1
#define SLOT1_DIRTY 2

static int slot_dirty_flags = 0;
static int write_slot = 0;
static uint32_t write_state;

Storage Storage::storage = {
    .ver = 0,
};

static const Storage storageInit = {
    .magic1 = STORAGE_MAGIC1,
	.ver = 0,
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
    },
	.crc = 0,
    .magic2 = STORAGE_MAGIC2,
};

uint32_t Storage::getCrc()
{
	Crc32 crc;
	auto old = storage.crc;
	storage.crc = 0;
	crc.data((uint8_t*)&storage, sizeof(storage));
	storage.crc = old;
	return crc.value;
}

void Storage::init()
{
	if (Storage::storage.magic1 == 0) {

		// Read and validate slot 0
		store_read(0, (uint8_t *)&storage, sizeof(storage));
		uint32_t slot0_ver = storage.ver;
		if (storage.magic1 != STORAGE_MAGIC1 || storage.magic2 != STORAGE_MAGIC2 || getCrc() != storage.crc) {
			DBG("Slot 0 invalid");
			slot_dirty_flags |= SLOT0_DIRTY;
		}

		// Read and validate slot 1
		store_read(1, (uint8_t *)&storage, sizeof(storage));
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
			store_read(0, (uint8_t *)&storage, sizeof(storage));
		} else if (slot_dirty_flags == (SLOT0_DIRTY | SLOT1_DIRTY)) { // If both are invalid, load default
			ERR("Loading default settings");
			memcpy(&storage, &storageInit, sizeof(storage));
			storage.crc = getCrc();
		} else {
			DBG("Using Slot 1");
		}
	}
}


void Storage::update()
{
	// If there are no dirty slots and write is not in progress, do nothing
	if (slot_dirty_flags == 0 && write_state == 0) return;

	if (write_state == 0) { // We are not currently writing, but we have new dirty slots
		// If current write_slot is not dirty, toggle the write_slot
		if (!(slot_dirty_flags & (1 << write_slot))) {
			write_slot ^= 1;
		}
		// Clear dirty flag, because we just starting write
		slot_dirty_flags &= ~(1 << write_slot);
		ERR("Writing to slot %d", write_slot);
	}

	store_write(&write_state, write_slot, (uint8_t*)&storage, sizeof(storage));
}


void Storage::write()
{
	storage.ver++;
	storage.crc = getCrc();
	slot_dirty_flags = (SLOT0_DIRTY | SLOT1_DIRTY);
}
