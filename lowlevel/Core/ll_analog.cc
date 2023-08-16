
#include "stm32l5xx_hal.h"
#include "main.h"
#include "lowlevel.hh"
#include "log.hh"

//#define DEBUG_SHOW_ADC_CHANNEL 8

static volatile uint16_t adc_values_dma[9];
static uint32_t adc_values[9];
static volatile bool adc_ready = false;


#ifdef DEBUG_SHOW_ADC_CHANNEL

#include "diag.hh"

static int get_temp(int x)
{
	//return (((312936 * x - 620439185) >> 11) * x - 46488827) >>	15; // KTY81/210 + 1.5K resistor on 12 bits ADC
	//return (((19559 * x - 620439186) >> 15) * x - 46488827) >>	15; // KTY81/210 + 1.5K resistor on 16 bits ADC
	//return (((400989 * x + 99792223 ) >> 12) * x - 336445037) >> 15; // KTY81/210 + 2.21K resistor on 12 bits ADC
	return (((25062 * x + 99792227 ) >> 16) * x - 336445037) >> 15; // KTY81/210 + 2.21K resistor on 16 bits ADC
}

void show_adc()
{
	static char buf[148];
	static char buf2[128];
	uint32_t raw = (uint32_t)adc_values_dma[DEBUG_SHOW_ADC_CHANNEL] * 16;
	uint32_t avg = adc_values[DEBUG_SHOW_ADC_CHANNEL] / 4096;
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

#endif // DEBUG_SHOW_ADC_CHANNEL


void handle_adc()
{
	for (uint32_t i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); i++) {
		adc_values[i] = ((uint32_t)adc_values[i] * 15 + 8) / 16 + adc_values_dma[i] * 4096;
	}
#ifdef DEBUG_SHOW_ADC_CHANNEL
	show_adc();
#endif
	auto res = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values_dma, sizeof(adc_values_dma) / sizeof(adc_values_dma[0]));
	if (res != HAL_OK) {
		ERR("ADC start error!");
		// TODO: go to fatal error state
	}
}

void init_adc()
{
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

	HAL_StatusTypeDef res = HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values_dma, sizeof(adc_values_dma) / sizeof(adc_values_dma[0]));
	while (res != HAL_OK) {} // TODO: go to fatal error state
	while (!adc_ready) {}

	for (uint32_t i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); i++) {
		adc_values[i] = adc_values_dma[i] * 16 * 4096;
	}
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
