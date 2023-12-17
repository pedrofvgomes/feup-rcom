#ifndef UTILS_H
#define UTILS_H

#include "../include/download.h"

int parse_url(char *input, struct Url *url);

int request_resource(const int socket, char *resource);

int get_resource(const int socketA, const int socketB, char *filename);

#endif /* UTILS_H */
