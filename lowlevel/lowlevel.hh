#ifndef _LOW_LEVEL_HH_
#define _LOW_LEVEL_HH_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SIMULATION_FULL 2
#define SIMULATION_ON_TARGET 1
#define SIMULATION_NONE 0

#ifndef SIMULATION_LEVEL
#define SIMULATION_LEVEL SIMULATION_NONE
#endif

#define TIME_BITS 31


//-------------------------------- ENVIRONMENT --------------------------------
/* Communicate with real word or a model.
 * Implementations:
 *   - model.cpp - use model.
 *   - ll_output.cc, ll_analog.cc, e.t.c. - use real word.
 */

// Gerneral
void env_init(); // Initialize low level before calling any other function
// Events
void startup_event(); // main() function is in lower level

// IO
void output(int index, bool state); // Relays and buzzer
bool input(int index); // ON/OFF with pull-up
uint32_t analog_input(int index); // Raw 16-bit value of analog input, converting and validating is done by upper level

// Time
uint32_t get_time(); // Absolute from startup, upper level is responsible for converting to 64-bits and overflow handling
void timeout(uint32_t t); // Absolute time, just one timeout at a time. Upper level is responsible for timeout queue.
#define PERIODIC_TIMEOUT 100 // Lower level will do timeouts in 10Hz interval independent from above function.
// Events
void timeout_event();


//-------------------------------- PERSISTENT DATA --------------------------------
/* Save data persistently.
 * Implementations:
 *   - storage-wasm.cpp - use WASM interface to store data in browser.
 *   - ll_store.cc - use flash on chip.
 */

void store_init();
void store_read(int slot, uint8_t* buffer, int size);
void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size); // Initial state==0, while state!=0 resume write later with the same parameters and state.


//-------------------------------- COMMUNICATION --------------------------------
/* Communicate with user interface SBC.
 * Implementations:
 *   - comm-wasm.cpp - use WASM interface to exchange data.
 *   - ll_comm.cc - use dedicated UART on chip.
 */

void comm_init();
int comm_free(); // Free space in communication FIFO
void comm_append(uint8_t data); // Append data to FIFO
void comm_send(); // Send appended data, just raw data, SLIP encoding in upper level
// Events
void comm_event(uint8_t* data, int size); // just raw data, SLIP decoding/validating/synch in upper level


//-------------------------------- DIAGNOSTICS --------------------------------
/* Communicate with developer.
 * Implementations:
 *   - none - connect directly to aux port.
 *   - model.cpp - use multiplexing with model interface over debug port.
 */

#if SIMULATION_LEVEL > SIMULATION_NONE

void diag_init();
int diag_free(); // Free space in diagnostics FIFO
void diag_append(uint8_t data); // Append data to FIFO
void diag_send(); // Send appended data, just raw data
// Events
void diag_event(uint8_t* data, int size); // just raw data

#else

static inline void diag_init() {
    aux_init();
}
static inline int diag_free() {
    return aux_free();
}
static inline void diag_append(uint8_t data) {
    aux_append(data);
}
static inline void diag_send() {
    aux_send();
}

// Events
void diag_event(uint8_t* data, int size); // just raw data

#endif


//-------------------------------- AUX PORT --------------------------------
/* Auxiliary port.
 * Implementations:
 *   - comm-wasm.cpp - use WASM interface to exchange data.
 *   - ll_comm.cc - use dedicated UART over USB.
 */

void aux_init();
int aux_free(); // Free space in aux FIFO
void aux_append(uint8_t data); // Append data to FIFO
void aux_append(uint8_t* data, int size); // Append data to FIFO
void aux_send(); // Send appended data, just raw data
void aux_flush(int freeSize); // Wait until specified free size is available
// Events
void aux_event(uint8_t* data, int size); // just raw data


//-------------------------------- FIRMWARE UPDATE --------------------------------
// for the future
//void update_init(); // Update firmware start
//void update_data(uint32_t* state, uint8_t* data, int size); // Append data to Firmware image
//bool update_exec(); // Execute firmware update (lower level is responsible for validating the image)
//void update_confirm(); // Confirm that currently running firmware is ok

#endif // _LOW_LEVEL_HH_
