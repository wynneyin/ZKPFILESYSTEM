#ifndef ZKP_UTILS_H
#define ZKP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "zkp_config.h"

void zkp_generate_uuid(char* uuid_str);
char* zkp_strdup(const char* str);
void zkp_secure_random(void* buf, size_t len);
int zkp_is_file_exists(const char* path);
char* zkp_get_timestamp_str(void);
void zkp_cleanup_temp_files(void);
ZKPStatus zkp_create_dir_if_not_exists(const char* path);

#endif /* ZKP_UTILS_H */