
#include <string.h>
#include "stm32l5xx_hal.h"
#include "main.h"
#include "lowlevel.hh"
#include "UartFifo.hh"


struct DiagMode {
    enum Type {
        LOGS,
        COMM,
        MENU,
    };
};

DiagMode::Type diagMode = DiagMode::LOGS;

UartFifo<256, 2048> diagUart(hlpuart1, hdma_lpuart1_rx, hdma_lpuart1_tx); // TODO: adjust queue sizes
UartFifo<1024, 2048> commUart(huart2, hdma_usart2_rx, hdma_usart2_tx);

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

extern "C"
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    diagUart.complete_callback(huart);
    commUart.complete_callback(huart);
}
