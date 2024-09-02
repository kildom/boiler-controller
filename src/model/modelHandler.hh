#ifndef MODEL_HANDLER_HH
#define MODEL_HANDLER_HH

#include <stdint.h>
#include <stddef.h>

#include "model.hh"

void modelEnsureStartup();

// Port for model handler
int modelPortFree(); // Free space in debug FIFO
bool modelPortIsEmpty(); // If sending buffer is empty
void modelPortAppend(uint8_t data); // Append data to FIFO
void modelPortAppend(const uint8_t* data, size_t size); // Append data to FIFO
void modelPortSend(); // Send appended data
uint32_t modelPortTime(); // Real time in milliseconds
void modelPortReset(); // Reset entire system

void modelPortEvent(const uint8_t* data, size_t size); // Event on data received for model handler

#endif
