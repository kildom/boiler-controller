
#include <string.h>

#include "stm32l5xx_ll_tim.h"
#include "stm32l5xx_hal_flash_ex.h"
#include "stm32l5xx_hal_flash.h"
#include "main.h"
#include "lowlevel.h"
#include "UartFifo.hh"

struct DiagMode {
	enum Type {
		LOGS,
		COMM,
		MENU,
	};
};

DiagMode::Type diagMode = DiagMode::LOGS;

UartFifo<256, 2048> diagUart(hlpuart1, hdma_lpuart1_rx, hdma_lpuart1_tx);
UartFifo<1024, 2048> commUart(huart2, hdma_usart2_rx, hdma_usart2_tx);


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	diagUart.complete_callback(huart);
	//commUart.complete_callback(huart);
}

uint32_t get_time()
{
	uint32_t cnt = LL_TIM_GetCounter(htim2.Instance);
	return cnt >> 1;
}

void timeout(uint32_t t)
{
	uint32_t cnt;
	uint32_t new_cnt = LL_TIM_GetCounter(htim2.Instance);

	t <<= 1;

	do {
		cnt = new_cnt;
		int32_t diff = t - cnt;
		if (diff < 1) {
			t = cnt + 1;
		}
		LL_TIM_OC_SetCompareCH1(htim2.Instance, t);
		new_cnt = LL_TIM_GetCounter(htim2.Instance);
	} while (cnt != new_cnt);
}

void output(int index, bool state)
{
}

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


bool store_write(uint32_t* state, int slot, const uint8_t* buffer, int size)
{

}


void store_read(uint8_t* buffer, int size)
{
	memcpy(buffer, (void*)store_begin, size);
}

void store_write(const uint8_t* buffer, int size)
{
	uint32_t pageError;
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.NbPages = store_pages;
	eraseInit.Page = store_page_begin;
	eraseInit.Banks = FLASH_BANK_2;
	HAL_FLASHEx_Erase(&eraseInit, &pageError);
	HAL_FLASH_Lock();
}


int comm_free()
{
	return commUart.free();
}

void comm_append(uint8_t data)
{
	return commUart.write(data);
}

void comm_send()
{
	commUart.send();
}

int diag_free()
{
	return diagUart.free();
}

void diag_append(uint8_t data)
{
	return diagUart.write(data);
}

void diag_send()
{
	diagUart.send();
}

void handle_uart_events()
{
	int size;
	uint8_t* buf;
	while ((buf = diagUart.read(&size))) {
		diag_event(buf, size);
	}
	while ((buf = commUart.read(&size))) {
		comm_event(buf, size);
	}
}
