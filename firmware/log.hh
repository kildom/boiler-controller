#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

extern char log_buffer[1024];

void log_send(int level);
const char* log_file_name(const char* file);
const char* log_time();

#define DBG(text, ...) _DEBUG2(__FILE__, __LINE__, text, ##__VA_ARGS__)
#define INF(text, ...) _INFO2(__FILE__, __LINE__, text, ##__VA_ARGS__)
#define ERR(text, ...) _ERROR2(__FILE__, __LINE__, text, ##__VA_ARGS__)

#define _DEBUG1(file, line, text, ...) do { snprintf(log_buffer, sizeof(log_buffer), "%s:%s:" #line ":dbg: " text, log_time(), log_file_name(file), ##__VA_ARGS__); log_send(0); } while (0)
#define _INFO1(file, line, text, ...) do { snprintf(log_buffer, sizeof(log_buffer), "%s:%s:" #line ":inf: " text, log_time(), log_file_name(file), ##__VA_ARGS__); log_send(1); } while (0)
#define _ERROR1(file, line, text, ...) do { snprintf(log_buffer, sizeof(log_buffer), "%s:%s:" #line ":err: " text, log_time(), log_file_name(file), ##__VA_ARGS__); log_send(2); } while (0)

#define _DEBUG2(file, line, text, ...) _DEBUG1(file, line, text, ##__VA_ARGS__)
#define _INFO2(file, line, text, ...) _INFO1(file, line, text, ##__VA_ARGS__)
#define _ERROR2(file, line, text, ...) _ERROR1(file, line, text, ##__VA_ARGS__)

#endif
