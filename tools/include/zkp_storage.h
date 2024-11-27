#ifndef ZKP_STORAGE_H
#define ZKP_STORAGE_H

#include <gmp.h>
#include "zkp_config.h"

#define ZKP_PROOF_ID_LENGTH 37
#define ZKP_MAX_PATH_LENGTH 256

typedef struct {
    char proof_id[ZKP_PROOF_ID_LENGTH];
    mpz_t commitment;
    mpz_t challenge;
    mpz_t response;
    time_t timestamp;
    char prover_id[64];
    char verifier_id[64];
    int round_number;
    int status;
} ProofRecord;

typedef struct {
    char* db_path;
    void* db_handle;
} ZKPStorage;

ZKPStatus zkp_storage_init(ZKPStorage* storage, const char* db_path);
ZKPStatus zkp_storage_close(ZKPStorage* storage);
ZKPStatus zkp_store_proof(ZKPStorage* storage, const ProofRecord* record);
ZKPStatus zkp_load_proof(ZKPStorage* storage, const char* proof_id, ProofRecord* record);
ZKPStatus zkp_delete_proof(ZKPStorage* storage, const char* proof_id);
ZKPStatus zkp_list_proofs(ZKPStorage* storage, ProofRecord** records, int* count);

#endif /* ZKP_STORAGE_H */