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

int openConnection(char *address, int port);

int login(const int socket, const char *user, const char *pass);

int passiveMode(const int socket, char* ip, int *port);

int readResponse(const int socket, char *buffer);

int closeConnection(const int socketA, const int socketB);