#include "../include/utils.h"

int parse(char *input, struct Url *url) {

    regex_t regex;
    regcomp(&regex, "/", 0);
    if (regexec(&regex, input, 0, NULL, 0)) return -1;

    regcomp(&regex, "@", 0);
    if (regexec(&regex, input, 0, NULL, 0) != 0) { //ftp://<host>/<url-path>
        
        sscanf(input, "%*[^/]//%[^/]", url->host);
        strcpy(url->user, DEFAULT_USER);
        strcpy(url->password, DEFAULT_PASSWORD);

    } else { // ftp://[<user>:<password>@]<host>/<url-path>
        
        sscanf(input, "%*[^/]//%*[^@]@%[^/]", url->host);
        sscanf(input, "%*[^/]//%[^:/]", url->user);
        sscanf(input, "%*[^/]//%*[^:]:%[^@\n$]", url->password);
    }

    sscanf(input, "%*[^/]//%*[^/]/%s", url->resource);
    strcpy(url->file, strrchr(input, '/') + 1);

    struct hostent *h;
    if (strlen(url->host) == 0) return -1;
    if ((h = gethostbyname(url->host)) == NULL) {
        printf("Invalid hostname '%s'\n", url->host);
        exit(-1);
    }
    strcpy(url->ip, inet_ntoa(*((struct in_addr *) h->h_addr)));

    return !(strlen(url->host) && strlen(url->user) && 
           strlen(url->password) && strlen(url->resource) && strlen(url->file));
}

int passiveMode(const int socket, char *ip, int *port) {

    char answer[MAX_LENGTH];
    int ip1, ip2, ip3, ip4, port1, port2;
    write(socket, "pasv\n", 5);
    if (readResponse(socket, answer) != SV_PASSIVE_MODE) return -1;

    sscanf(answer, "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    *port = port1 * 256 + port2;
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    return SV_PASSIVE_MODE;
}

int readResponse(const int socket, char* buffer) {

    char byte;
    int index = 0, responseCode;
    ResponseState state = START;
    memset(buffer, 0, MAX_LENGTH);

    while (state != END) {
        
        read(socket, &byte, 1);
        switch (state) {
            case START:
                if (byte == ' ') state = SINGLE;
                else if (byte == '-') state = MULTIPLE;
                else if (byte == '\n') state = END;
                else buffer[index++] = byte;
                break;
            case SINGLE:
                if (byte == '\n') state = END;
                else buffer[index++] = byte;
                break;
            case MULTIPLE:
                if (byte == '\n') {
                    memset(buffer, 0, MAX_LENGTH);
                    state = START;
                    index = 0;
                }
                else buffer[index++] = byte;
                break;
            case END:
                break;
            default:
                break;
        }
    }

    sscanf(buffer, "%d", &responseCode);
    return responseCode;
}

int requestResource(const int socket, char *resource) {

    char fileCommand[5+strlen(resource)+1], answer[MAX_LENGTH];
    sprintf(fileCommand, "retr %s\n", resource);
    write(socket, fileCommand, sizeof(fileCommand));
    return readResponse(socket, answer);
}

int getResource(const int socketA, const int socketB, char *filename) {

    FILE *fd = fopen(filename, "wb");
    if (fd == NULL) {
        printf("Error opening or creating file '%s'\n", filename);
        exit(-1);
    }

    char buffer[MAX_LENGTH];
    int bytes;

    do {
        bytes = read(socketB, buffer, MAX_LENGTH);
        if (fwrite(buffer, bytes, 1, fd) < 0) return -1;
    } while (bytes);
    fclose(fd);

    return readResponse(socketA, buffer);
}
