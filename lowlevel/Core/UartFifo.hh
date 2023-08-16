#ifndef UARTFIFO_HH_
#define UARTFIFO_HH_

#include "global.hh"

#include "stm32l5xx_hal_dma.h"
#include "stm32l5xx_hal_uart.h"

template <int rx_size, int tx_size>
class UartFifo {
private:
	UART_HandleTypeDef& uart;
	DMA_HandleTypeDef& dma_rx;
	DMA_HandleTypeDef& dma_tx;

	uint8_t* rx_read_ptr;

	uint8_t* volatile tx_read_ptr;
	uint8_t* volatile tx_write_ptr;

	uint32_t busy;

	uint8_t rx_buf[rx_size];
	uint8_t tx_buf[tx_size];

public:
	UartFifo(UART_HandleTypeDef& uart, DMA_HandleTypeDef& dma_rx, DMA_HandleTypeDef& dma_tx) :
		uart(uart),
		dma_rx(dma_rx),
		dma_tx(dma_tx),
		rx_read_ptr(rx_buf),
		tx_read_ptr(tx_buf),
		tx_write_ptr(tx_buf),
		busy(0)
	{
		  HAL_UART_Receive_DMA(&uart, rx_buf, rx_size);
	}

	uint8_t* read(int* size);
	int free();
	void write(uint8_t data)
	{
		*tx_write_ptr++ = data;
		if (tx_write_ptr == &tx_buf[tx_size]) {
			tx_write_ptr = tx_buf;
		}
	}
	void send();
	void complete_callback(UART_HandleTypeDef* src_uart) {
		if (src_uart == &uart) {
			__atomic_fetch_and(&busy, ~1, __ATOMIC_SEQ_CST); // atomic store?
			send();
		}
	}
};

template <int rx_size, int tx_size>
uint8_t* UartFifo<rx_size, tx_size>::read(int* size)
{
	int index = (rx_size - dma_rx.Instance->CNDTR) % rx_size;
	uint8_t* result = rx_read_ptr;
	uint8_t* write_ptr = &rx_buf[index];
	if (rx_read_ptr == write_ptr) {
		*size = 0;
		return NULL;
	} else if (rx_read_ptr < write_ptr) {
		*size = write_ptr - rx_read_ptr;
		rx_read_ptr = write_ptr;
		return result;
	} else {
		*size = rx_size - (rx_read_ptr - write_ptr);
		rx_read_ptr = rx_buf;
		return result;
	}
}

template <int rx_size, int tx_size>
int UartFifo<rx_size, tx_size>::free()
{
	uint8_t* write_ptr = tx_write_ptr;
	uint8_t* read_ptr = tx_read_ptr;
	int sub = 1 + dma_tx.Instance->CNDTR;
	if (read_ptr <= write_ptr) {
		return tx_size - (write_ptr - read_ptr) - sub;
	} else {
		return read_ptr - write_ptr - sub;
	}
}

template <int rx_size, int tx_size>
void UartFifo<rx_size, tx_size>::send()
{
	uint8_t* write_ptr = tx_write_ptr;
	uint8_t* read_ptr = tx_read_ptr;
	if (read_ptr == write_ptr) return;
	uint32_t old = __atomic_fetch_or(&busy, 1, __ATOMIC_SEQ_CST); // TODO: atomic store??
	if (old != 0) return;
	if (read_ptr < write_ptr) {
		HAL_UART_Transmit_DMA(&uart, read_ptr, write_ptr - read_ptr);
		tx_read_ptr = write_ptr;
	} else {
		HAL_UART_Transmit_DMA(&uart, read_ptr, tx_buf + tx_size - read_ptr);
		tx_read_ptr = tx_buf;
	}
}

#endif
