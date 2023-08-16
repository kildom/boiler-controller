
#include <stdint.h>
#include <string.h>
#include "log.hh"
#include "time.hh"
#include "proto.hh"
#include "diag.hh"

#define MAX_LEVEL 2

char log_buffer[1024];
static uint8_t dropped[MAX_LEVEL + 1] = { 0, 0, 0 };

void log_send(int level)
{
    Diag::log(log_buffer);
    size_t len = strlen(log_buffer);
    size_t free = Proto::available();
    if (1 + sizeof(dropped) + len > free) {
        dropped[level]++;
        if (dropped[level] == 0) {
            dropped[level] = 255;
        }
        if (1 + sizeof(dropped) > free) {
            return;
        } else {
            len = 0;
        }
    }
    Proto::start(Proto::LOG);
    Proto::byte(level);
    Proto::data(dropped, sizeof(dropped));
    Proto::data((uint8_t*)log_buffer, len);
    Proto::end();
    memset(dropped, 0, sizeof(dropped));
}

const char* log_file_name(const char* file)
{
    const char* a = strrchr(file, '/');
    const char* b = strrchr(file, '\\');
    const char* last;
    if (a == NULL || b > a) {
        last = b;
    } else {
        last = a;
    }
    if (last == NULL) {
        return file;
    }
    return last + 1;
}

const char* log_time()
{
    static char buf[16];
    int time = Time::real_time() % (1000 * 60 * 60 * 24);
    int ms = time % 1000;
    time /= 1000;
    int sec = time % 60;
    time /= 60;
    int min = time % 60;
    time /= 60;
    int h = time;
    sprintf(buf, "%02d:%02d:%02d.%03d", h, min, sec, ms);
    return buf;
}
