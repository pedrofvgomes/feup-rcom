#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <termios.h>

#include "../include/download.h"

int parse(char *input, struct Url *url);

int requestResource(const int socket, char *resource);

int getResource(const int socketA, const int socketB, char *filename);
