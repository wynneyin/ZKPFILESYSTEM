#ifndef ZKP_LOG_H
#define ZKP_LOG_H

#include <stdarg.h>
#include "zkp_config.h"

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

typedef struct {
    char* log_file;
    LogLevel min_level;
    int enable_console;
    int enable_file;
} ZKPLogger;

ZKPStatus zkp_log_init(const char* log_file);
void zkp_log(LogLevel level, const char* format, ...);
void zkp_log_hexdump(LogLevel level, const char* desc, const void* addr, int len);
void zkp_log_close(void);
void zkp_log_set_level(LogLevel level);
const char* zkp_log_level_string(LogLevel level);

#endif /* ZKP_LOG_H */