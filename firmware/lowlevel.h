
#include <stdint.h>
#include <stdbool.h>

#define RGB_LED_COUNT 8

// UPPER LEVELS ==> LOW LEVEL

// IO
void output(int index, bool state); // Relays and buzzer
uin32_t analog_input(int index); // Raw value of analog input, converting and validating is done by upper level

// Time
uint32_t time(); // Absolute from startup, upper level is responsible for converting to 64-bits and overflow handling
void timeout(uint32_t t); // Absolute time, just one timeout at a time (upper level is responsible for timeout queue)

// Persistent data
void store_read(uint8_t* buffer, int size);
void store_write(uint8_t* buffer, int size);

// RGB LEDs
extern uint32_t rgb_color[RGB_LED_COUNT];
void rgb_update(); // send rgb_color to LEDs

// Communication
int comm_free(); // Free space in communication FIFO
void comm_append(uint8_t* data, int size); // Append data to FIFO
void comm_send(); // Send appended data, just raw data, SLIP encoding in upper level

// LOW LEVEL ==> UPPER LEVELS

void startup_event(); // main() function is in lower level
void button_event(int index, bool state); // upper level will count time for long press
void timeout_event();
void comm_event(uint8_t* data, int size); // just raw data, SLIP decoding/validating/synch in upper level
