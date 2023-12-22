#ifndef CONNECTION_H
#define CONNECTION_H

#include "download.h"
#include "utils.h"

int open_connection(char *address, int port);

int login(const int socket, const char *user, const char *password);

int passive_mode(const int socket, char* ip, int *port);

int read_response(const int socket, char *buffer);

int close_connection(const int control_socket, const int data_socket);

#endif /* UTILS_H */
