#ifndef ZKP_CONFIG_H
#define ZKP_CONFIG_H

#include <time.h>

#define ZKP_VERSION "1.0.0"
#define ZKP_MAX_ROUNDS 100
#define ZKP_DEFAULT_TIMEOUT 30
#define ZKP_DEFAULT_PRIME_BITS 2048

typedef struct {
    int security_level;    
    int rounds;           
    char *prime_bits;     
    int timeout;         
    char *log_file;      
    char *db_path;       
} ZKPConfig;

typedef enum {
    ZKP_SUCCESS = 0,
    ZKP_ERROR_INIT = -1,
    ZKP_ERROR_PARAMS = -2,
    ZKP_ERROR_PROOF = -3,
    ZKP_ERROR_TIMEOUT = -4,
    ZKP_ERROR_MEMORY = -5,
    ZKP_ERROR_IO = -6,
    ZKP_ERROR_NETWORK = -7
} ZKPStatus;

ZKPConfig* zkp_config_new(void);
void zkp_config_free(ZKPConfig* config);
ZKPStatus zkp_config_load(ZKPConfig* config, const char* config_file);
ZKPStatus zkp_config_save(const ZKPConfig* config, const char* config_file);

#endif /* ZKP_CONFIG_H */