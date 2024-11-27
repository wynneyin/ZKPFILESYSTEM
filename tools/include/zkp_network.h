#ifndef ZKP_NETWORK_H
#define ZKP_NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <gmp.h>
#include "zkp_config.h"

#define ZKP_DEFAULT_PORT 8888
#define ZKP_MAX_CONNECTIONS 10
#define ZKP_BUFFER_SIZE 4096

typedef struct {
    int socket_fd;
    struct sockaddr_in addr;
    int is_server;
    int port;
    char* host;
    int timeout;
} ZKPConnection;

typedef struct {
    ZKPConnection* conn;
    void* ssl;           // 预留给SSL/TLS
    int secure;
} ZKPSession;

ZKPStatus zkp_network_init(ZKPConnection* conn, const char* host, int port, int is_server);
ZKPStatus zkp_network_connect(ZKPConnection* conn);
ZKPStatus zkp_network_accept(ZKPConnection* server, ZKPConnection* client);
ZKPStatus zkp_send_mpz(ZKPConnection* conn, const mpz_t value);
ZKPStatus zkp_receive_mpz(ZKPConnection* conn, mpz_t value);
ZKPStatus zkp_send_proof(ZKPConnection* conn, const ProofRecord* record);
ZKPStatus zkp_receive_proof(ZKPConnection* conn, ProofRecord* record);
void zkp_network_close(ZKPConnection* conn);

#endif /* ZKP_NETWORK_H */