#ifndef ZKP_CRYPTO_H
#define ZKP_CRYPTO_H

#include <gmp.h>
#include "zkp_config.h"

typedef struct {
    mpz_t p;       // 大素数
    mpz_t q;       // (p-1)/2，也是素数
    mpz_t g;       // 生成元
    int bits;      // 素数位数
} ZKPParams;

typedef struct {
    ZKPParams* params;
    mpz_t secret;    
    mpz_t public_key;
} ZKPKeyPair;

ZKPStatus zkp_params_init(ZKPParams* params, int bits);
ZKPStatus zkp_params_free(ZKPParams* params);
ZKPStatus zkp_generate_keypair(ZKPParams* params, ZKPKeyPair* keypair);
ZKPStatus zkp_keypair_free(ZKPKeyPair* keypair);
ZKPStatus zkp_random_prime(mpz_t prime, int bits);
ZKPStatus zkp_random_safe_prime(mpz_t prime, int bits);
ZKPStatus zkp_find_generator(const mpz_t p, const mpz_t q, mpz_t g);

#endif /* ZKP_CRYPTO_H */