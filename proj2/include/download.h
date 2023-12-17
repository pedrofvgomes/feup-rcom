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

//#include "connection.h"

#define MAX_LENGTH  500
#define FTP_PORT    21

/* Server responses */
#define SV_READY_FOR_NEW_USER    220
#define SV_READY_FOR_PASSWORD    331
#define SV_LOGIN_SUCCESSFUL      230
#define SV_PASSIVE_MODE          227
#define SV_READY4TRANSFER        150
#define SV_TRANSFER_COMPLETE     226
#define SV_END                   221

/* Parser regular expressions */
#define AT              "@"
#define BAR             "/"
#define HOST_REGEX      "%*[^/]//%[^/]"
#define HOST_AT_REGEX   "%*[^/]//%*[^@]@%[^/]"
#define RESOURCE_REGEX  "%*[^/]//%*[^/]/%s"
#define USER_REGEX      "%*[^/]//%[^:/]"
#define PASS_REGEX      "%*[^/]//%*[^:]:%[^@\n$]"
#define RESPCODE_REGEX  "%d"
#define PASSIVE_REGEX   "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]"

/* Default login for case 'ftp://<host>/<url-path>' */
#define DEFAULT_USER        "anonymous"
#define DEFAULT_PASSWORD    "password"


struct URL {
    char host[MAX_LENGTH];      // 'ftp.up.pt'
    char resource[MAX_LENGTH];  // 'parrot/misc/canary/warrant-canary-0.txt'
    char file[MAX_LENGTH];      // 'warrant-canary-0.txt'
    char user[MAX_LENGTH];      // 'username'
    char password[MAX_LENGTH];  // 'password'
    char ip[MAX_LENGTH];        // 193.137.29.15
};

typedef enum {
    START,
    SINGLE,
    MULTIPLE,
    END
} ResponseState;

int openConnection(char *ip, int port);

int login(const int socket, const char *user, const char *pass);

int closeConnection(const int socketA, const int socketB);

int parse(char *input, struct URL *url);

int readResponse(const int socket, char *buffer);

int passiveMode(const int socket, char* ip, int *port);

int requestResource(const int socket, char *resource);

int getResource(const int socketA, const int socketB, char *filename);

