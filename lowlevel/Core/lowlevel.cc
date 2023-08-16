
#include <string.h>
#include <algorithm>

#include "stm32l5xx_ll_tim.h"
#include "stm32l5xx_hal_flash_ex.h"
#include "stm32l5xx_hal_flash.h"
#include "main.h"
#include "log.hh"
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

uint32_t sysTickEvent = 0;


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
		} else if (diff > PERIODIC_TIMEOUT) {
			return;
		}
		LL_TIM_OC_SetCompareCH1(htim2.Instance, t);
		new_cnt = LL_TIM_GetCounter(htim2.Instance);
	} while (cnt != new_cnt);
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

struct WriteStage {
	enum Type {
		INIT = 0,
		ERASING_PAGE = 1,
		PROGRAMMING = 2,
	};
};

static const int PROGRAM_WORD_SIZE = 8;

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

static volatile bool timerCaptured = true;

extern "C"
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	timerCaptured = true;
}

#include "time.hh"

volatile uint16_t adc_values_dma[9];
uint32_t adc_values[9];
volatile bool adc_ready = false;

void handle_adc()
{
	for (uint32_t i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); i++) {
		adc_values[i] = ((uint32_t)adc_values[i] * 15 + 8) / 16 + adc_values_dma[i] * 4096;
	}
	auto res = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values_dma, sizeof(adc_values_dma) / sizeof(adc_values_dma[0]));
	if (res != HAL_OK) {
		ERR("ADC start error!");
		// TODO: go to fatal error state
	}
}


static int get_temp(int x)
{
	//return (((312936 * x - 620439185) >> 11) * x - 46488827) >>	15; // KTY81/210 + 1.5K resistor on 12 bits ADC
	//return (((19559 * x - 620439186) >> 15) * x - 46488827) >>	15; // KTY81/210 + 1.5K resistor on 16 bits ADC
	//return (((400989 * x + 99792223 ) >> 12) * x - 336445037) >> 15; // KTY81/210 + 2.21K resistor on 12 bits ADC
	return (((25062 * x + 99792227 ) >> 16) * x - 336445037) >> 15; // KTY81/210 + 2.21K resistor on 16 bits ADC
}

#include "diag.hh"

void show_adc()
{
	static char buf[148];
	static char buf2[128];
	uint32_t raw = (uint32_t)adc_values_dma[8] * 16;
	uint32_t avg = adc_values[8] / 4096;
	auto r = get_temp(raw);
	auto t = get_temp(avg);
	memset(buf2, ' ', 100);
	buf2[100] = 0;
	buf2[(uint32_t)r % 100] = '.';
	buf2[(uint32_t)t % 100] = 'o';
	if (r == t) buf2[t % 100] = '#';
	sprintf(buf, "%d %d %d %d |%s|\r\n", raw, r, avg, t, buf2);
	Diag::write(buf, -1, Diag::MENU);
}

uint32_t analog_input(int index)
{
	// TODO: ASSERT(index < sizeof(adc_values) / sizeof(adc_values[0]))
	return adc_values[index] / 4096;
}

extern "C"
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	adc_ready = true;
}

struct RelayConf {
	GPIO_TypeDef *port;
	uint16_t pin;
};

static const RelayConf relayConf[16] = {
	{ RELAY0_GPIO_Port, RELAY0_Pin },
	{ RELAY1_GPIO_Port, RELAY1_Pin },
	{ RELAY2_GPIO_Port, RELAY2_Pin },
	{ RELAY3_GPIO_Port, RELAY3_Pin },
	{ RELAY4_GPIO_Port, RELAY4_Pin },
	{ RELAY5_GPIO_Port, RELAY5_Pin },
	{ RELAY6_GPIO_Port, RELAY6_Pin },
	{ RELAY7_GPIO_Port, RELAY7_Pin },
	{ RELAY8_GPIO_Port, RELAY8_Pin },
	{ RELAY9_GPIO_Port, RELAY9_Pin },
	{ RELAY10_GPIO_Port, RELAY10_Pin },
	{ RELAY11_GPIO_Port, RELAY11_Pin },
	{ RELAY12_GPIO_Port, RELAY12_Pin },
	{ RELAY13_GPIO_Port, RELAY13_Pin },
	{ RELAY14_GPIO_Port, RELAY14_Pin },
	{ RELAY15_GPIO_Port, RELAY15_Pin },
};

void output(int index, bool state)
{
	// TODO: ASSERT(index < sizeof(relayConf) / sizeof(relayConf[0]))
	HAL_GPIO_WritePin(relayConf[index].port, relayConf[index].pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


void main_loop()
{

	// Wait for event (exit immediately if already set).
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	__WFE();
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

	auto oldSysTickEvent = __atomic_exchange_n(&sysTickEvent, 0, __ATOMIC_SEQ_CST);

	// The timeout event
	if (LL_TIM_IsActiveFlag_CC1(htim2.Instance) || timerCaptured || oldSysTickEvent) {
		LL_TIM_ClearFlag_CC1(htim2.Instance);
		timerCaptured = false;
		// Call timeout callback.
		timeout_event();
	}

	handle_uart_events();

	if (oldSysTickEvent) {
		show_adc();
		handle_adc();
	}

	/*int comm_size;
	uint8_t* comm_buffer;

	while ((comm_buffer = read_comm_fifo(&comm_size))) {
		comm_event(comm_buffer, comm_size);
		sprintf((char*)buf, "%d\r\n", comm_size);
		HAL_UART_Transmit_DMA(&hlpuart1, buf, strlen(buf));
	}*/

	//HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    //HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	//for (volatile int i = 0; i < 10000000; i++);
	/*uint32_t cnt = LL_TIM_GetCounter(htim2.Instance);
	cnt /= 2000;
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, (cnt & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, LL_TIM_IsActiveFlag_CC1(htim2.Instance) ? GPIO_PIN_SET : GPIO_PIN_RESET);*/
}

