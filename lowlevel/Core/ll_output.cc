
#include "stm32l5xx_hal.h"
#include "main.h"
#include "lowlevel.hh"

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
