#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "connection.h"
#include "utils.h"

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

#define BUFFER_SIZE 256
#define FTP_PORT    21

#define FTP_USAGE "Usage: ./app ftp://[<user>:<password>@]<host>/<url-path>\n"

#define SV_READY_FOR_NEW_USER    220
#define SV_READY_FOR_PASSWORD    331
#define SV_LOGIN_SUCCESSFUL      230
#define SV_PASSIVE_MODE          227
#define SV_READY_TRANSFER        150
#define SV_TRANSFER_COMPLETE     226
#define SV_END                   221

#define DEFAULT_USER        "anonymous"
#define DEFAULT_PASSWORD    "password"

typedef enum {
    START,
    SINGLE,
    MULTIPLE,
    END
} ResponseState;

void handle_error(const char* message);

void download_FTP_file(const char* url_name);

#endif /* DOWNLOAD_H */
