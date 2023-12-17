#ifndef DOWNLOAD_H
#define DOWNLOAD_H

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
#include <time.h>

#define MAX_LENGTH  500
#define BUFFER_SIZE 256
#define FTP_PORT    21

#define FTP_USAGE "Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n"

/* Server responses */
#define SV_READY_FOR_NEW_USER    220
#define SV_READY_FOR_PASSWORD    331
#define SV_LOGIN_SUCCESSFUL      230
#define SV_PASSIVE_MODE          227
#define SV_READY_TRANSFER        150
#define SV_TRANSFER_COMPLETE     226
#define SV_END                   221

/* Default login for case 'ftp://<host>/<url-path>' */
#define DEFAULT_USER        "anonymous"
#define DEFAULT_PASSWORD    "password"


struct Url {
    char user[MAX_LENGTH];      
    char password[MAX_LENGTH]; 
    char host[MAX_LENGTH];      
        char ip[MAX_LENGTH];        
    char resource[MAX_LENGTH]; 
    char file[MAX_LENGTH];      
};

typedef enum {
    START,
    SINGLE,
    MULTIPLE,
    END
} ResponseState;

int parse_url(char *input, struct Url *url);
int open_connection(char *address, int port);

#endif /* DOWNLOAD_H */
