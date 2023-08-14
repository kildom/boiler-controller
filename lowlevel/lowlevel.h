
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_BITS 31

#define RGB_LED_COUNT 8

// UPPER LEVELS ==> LOW LEVEL

// Gerneral
void global_init(); // Initialize low level before calling any other function

// IO
void output(int index, bool state); // Relays and buzzer
uint32_t analog_input(int index); // Raw 16-bit value of analog input, converting and validating is done by upper level

// Time
uint32_t get_time(); // Absolute from startup, upper level is responsible for converting to 64-bits and overflow handling
void timeout(uint32_t t); // Absolute time, just one timeout at a time (upper level is responsible for timeout queue)

// Persistent data
void store_read(int slot, uint8_t* buffer, int size);
void store_write(uint32_t* state, int slot, const uint8_t* buffer, int size); // Initial state==0, while state!=0 resume write later with the same parameters and state.

// RGB LEDs
extern uint32_t rgb_color[RGB_LED_COUNT];
void rgb_update(); // send rgb_color to LEDs

// Communication
int comm_free(); // Free space in communication FIFO
void comm_append(uint8_t data); // Append data to FIFO
void comm_send(); // Send appended data, just raw data, SLIP encoding in upper level

// Diagnostics port
int diag_free(); // Free space in diagnostics FIFO
void diag_append(uint8_t data); // Append data to FIFO
void diag_send(); // Send appended data, just raw data

// Firmware update
void update_init(); // Update firmware start
void update_data(uint8_t* data, int size); // Append data to Firmware image
bool update_exec(); // Execute firmware update (lower level is responsible for validating the image)
void update_confirm(); // Confirm that currently running firmware is ok

// LOW LEVEL ==> UPPER LEVELS

void startup_event(); // main() function is in lower level
void button_event(int index, bool state); // upper level will count time for long press
void timeout_event();
void comm_event(uint8_t* data, int size); // just raw data, SLIP decoding/validating/synch in upper level
void diag_event(uint8_t* data, int size); // just raw data

#ifdef __cplusplus
}
#endif
