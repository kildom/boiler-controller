
#include <string.h>
#include <algorithm>
#include "stm32l5xx_hal.h"
#include "main.h"
#include "lowlevel.hh"

struct WriteStage {
    enum Type {
        INIT = 0,
        ERASING_PAGE = 1,
        PROGRAMMING = 2,
    };
};

static const int PROGRAM_WORD_SIZE = 8;
static const uint32_t store_page_end = FLASH_SIZE / FLASH_PAGE_SIZE;


void store_read(int slot, uint8_t* buffer, int size)
{
    const uint32_t store_pages = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    const uint32_t store_begin[2] = {
        FLASH_BASE + (store_page_end - store_pages) * FLASH_PAGE_SIZE,
        FLASH_BASE + (store_page_end - 2 * store_pages) * FLASH_PAGE_SIZE,
    };
    memcpy(buffer, (void*)store_begin[slot], size);
}


void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size)
{
    const uint32_t store_pages = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    const uint32_t store_page_begin[2] = {
        (store_page_end - store_pages),
        (store_page_end - 2 * store_pages),
    };
    const uint32_t store_begin[2] = {
        FLASH_BASE + (store_page_end - store_pages) * FLASH_PAGE_SIZE,
        FLASH_BASE + (store_page_end - 2 * store_pages) * FLASH_PAGE_SIZE,
    };
    WriteStage::Type stage = (WriteStage::Type)(*state >> 24);
    int address = *state & 0x7FFFFF;

    if (FLASH_WaitForLastOperation(0) != HAL_OK) {
        goto save_state_and_exit;
    }

    if (stage == WriteStage::INIT) {
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
        HAL_FLASH_Unlock();
        stage = WriteStage::ERASING_PAGE;
        address = -1;
    }

    if (stage == WriteStage::ERASING_PAGE) {
        address++;
        if (address >= (int)store_pages) {
            stage = WriteStage::PROGRAMMING;
            address = -PROGRAM_WORD_SIZE;
        } else {
            FLASH_EraseInitTypeDef eraseInit;
            eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
            eraseInit.NbPages = 1;
            eraseInit.Page = store_page_begin[slot] + address;
            eraseInit.Banks = FLASH_BANK_2;
            HAL_FLASHEx_Erase_IT(&eraseInit);
            goto save_state_and_exit;
        }
    }

    if (stage == WriteStage::PROGRAMMING) {
        address += PROGRAM_WORD_SIZE;
        if (address >= size) {
            HAL_FLASH_Lock();
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
            *state = 0;
            return;
        } else {
            uint64_t data = 0;
            auto this_size = std::min(PROGRAM_WORD_SIZE, size - address);
            memcpy(&data, &buffer[address], this_size);
            HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_DOUBLEWORD, store_begin[slot] + address, data);
        }
    }

save_state_and_exit:
    *state = address | ((uint32_t)stage << 24) | 0x800000;
}
