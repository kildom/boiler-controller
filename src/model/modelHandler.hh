#ifndef MODEL_HANDLER_HH
#define MODEL_HANDLER_HH

#include <stdint.h>
#include <stddef.h>

#include "model.hh"

extern State modelState;

// Called on incoming data over debug port
void modelDataOnReceived(const uint8_t* data, size_t size);
// Send data over debug port
void modelDataSend(const uint8_t* data, size_t size);

// Get comm port incoming data (used by model handler to call comm_event())
size_t modelCommReceive(const uint8_t** data);

// Debugging port
int debugPortFree(); // Free space in debug FIFO
void debugPortAppend(uint8_t data); // Append data to FIFO
void debugPortAppend(const uint8_t* data, size_t size); // Append data to FIFO
void debugPortSend(); // Send appended data

#endif
