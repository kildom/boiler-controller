
#include "stm32l5xx_hal.h"
#include "main.h"
#include "lowlevel.hh"
#include "ll_time.hh"
#include "ll_comm.hh"
#include "ll_analog.hh"


uint32_t sysTickEvent = 0;


void global_init()
{
	main_construct_init();
}

void gpio_post_init()
{
	for (int i = 0; i < 16; i++) { // TODO: 16 should be define
		output(i, false);
	}
}

void main_post_init()
{
	// Enable automatic calibration
	HAL_RCCEx_EnableMSIPLLMode();

	// Start timer
	init_time();

	// Init ADC
	init_adc();

	// Lower the sys tick frequency
	HAL_SetTickFreq(HAL_TICK_FREQ_10HZ);

	// Disable deep sleep
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}


void main_pre_loop()
{
	startup_event();
}

void main_loop()
{
	// Wait for event (exit immediately if already set).
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	__WFE();
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

	// Check if sys tick occured
	auto oldSysTickEvent = __atomic_exchange_n(&sysTickEvent, 0, __ATOMIC_SEQ_CST);

	// The any timeout event occured
	if (is_timeout() || oldSysTickEvent) {
		// Call timeout callback.
		timeout_event();
	}

	// Handle UART communication always
	handle_uart_events();

	// Handler ADC on sys tick
	if (oldSysTickEvent) {
		handle_adc();
	}
}

