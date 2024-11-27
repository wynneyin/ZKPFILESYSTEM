#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include "include/zkp_config.h"
#include "include/zkp_log.h"
#include "include/zkp_storage.h"
#include "include/zkp_network.h"
#include "include/zkp_crypto.h"
#include "include/zkp_utils.h"
typedef struct {
    mpz_t secret;    
    mpz_t g;         
    mpz_t p;         
    mpz_t y;         
} Prover;

typedef struct {
    mpz_t g;         
    mpz_t p;         
    mpz_t y;         
} Verifier;

void prover_init(Prover *p) {
    mpz_init(p->secret);
    mpz_init(p->g);
    mpz_init(p->p);
    mpz_init(p->y);
    
    mpz_set_str(p->p, "23", 10);
    mpz_set_str(p->g, "5", 10);
    mpz_set_str(p->secret, "6", 10);
    
    mpz_powm(p->y, p->g, p->secret, p->p);
}

void verifier_init(Verifier *v, mpz_t g, mpz_t p, mpz_t y) {
    mpz_init_set(v->g, g);
    mpz_init_set(v->p, p);
    mpz_init_set(v->y, y);
}

void prover_step1(Prover *p, mpz_t r, mpz_t t) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    mpz_init(r);
    mpz_urandomm(r, state, p->p);
    
    mpz_init(t);
    mpz_powm(t, p->g, r, p->p);
    
    gmp_randclear(state);
}

void verifier_challenge(mpz_t c) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    mpz_init(c);
    mpz_set_ui(c, rand() % 2);
    
    gmp_randclear(state);
}

void prover_response(Prover *p, mpz_t r, mpz_t c, mpz_t s) {
    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, p->p, 1);
    
    mpz_init(s);
    mpz_mul(s, c, p->secret);
    mpz_add(s, s, r);
    mpz_mod(s, s, temp);
    
    mpz_clear(temp);
}

int verifier_verify(Verifier *v, mpz_t t, mpz_t c, mpz_t s) {
    mpz_t left, right;
    mpz_init(left);
    mpz_init(right);
    
    mpz_powm(left, v->g, s, v->p);
    mpz_powm(right, v->y, c, v->p);
    mpz_mul(right, right, t);
    mpz_mod(right, right, v->p);
    
    int result = (mpz_cmp(left, right) == 0);
    
    mpz_clear(left);
    mpz_clear(right);
    
    return result;
}

int main() {
    Prover prover;
    Verifier verifier;
    mpz_t r, t, c, s;
    
    prover_init(&prover);
    verifier_init(&verifier, prover.g, prover.p, prover.y);
    
    printf("开始零知识证明协议...\n");
    
    prover_step1(&prover, r, t);
    verifier_challenge(c);
    prover_response(&prover, r, c, s);
    
    if (verifier_verify(&verifier, t, c, s)) {
        printf("验证成功！\n");
    } else {
        printf("验证失败！\n");
    }
    
    mpz_clear(r);
    mpz_clear(t);
    mpz_clear(c);
    mpz_clear(s);
    
    return 0;
}