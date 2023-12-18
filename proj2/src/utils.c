#include "../include/utils.h"

int parse_url(char *input, struct Url *url) {

    regex_t regex;
    regcomp(&regex, "/", 0);
    if (regexec(&regex, input, 0, NULL, 0)) 
        return -1;

    regcomp(&regex, "@", 0);

    // uses default user and password

    if (regexec(&regex, input, 0, NULL, 0) != 0) { 
        sscanf(input, "%*[^/]//%[^/]", url->host);
        strcpy(url->user, DEFAULT_USER);
        strcpy(url->password, DEFAULT_PASSWORD);
    } 

   // specific user and password

    else { 
        
        sscanf(input, "%*[^/]//%*[^@]@%[^/]", url->host);
        sscanf(input, "%*[^/]//%[^:/]", url->user);
        sscanf(input, "%*[^/]//%*[^:]:%[^@\n$]", url->password);
    }

    sscanf(input, "%*[^/]//%*[^/]/%s", url->resource);
    strcpy(url->file, strrchr(input, '/') + 1);

    struct hostent *h;
    if (strlen(url->host) == 0) 
        return -1;

    if ((h = gethostbyname(url->host)) == NULL) {
        printf("Error i=on hostname '%s'\n", url->host);
        exit(-1);
    }
    strcpy(url->ip, inet_ntoa(*((struct in_addr *) h->h_addr)));

    return !(strlen(url->host) && strlen(url->user) && strlen(url->password) && strlen(url->resource) && strlen(url->file));
}

int request_resource(const int socket, char *resource) {

    char file_request[5+strlen(resource)+1], answer[MAX_LENGTH];
    sprintf(file_request, "retr %s\n", resource);
    write(socket, file_request, sizeof(file_request));
    return read_response(socket, answer);
}

int get_resource(const int socketA, const int socketB, char *filename) {

    FILE *fd = fopen(filename, "wb");
    if (fd == NULL) {
        printf("Error opening file '%s'\n", filename);
        exit(-1);
    }

    char buffer[MAX_LENGTH];
    int bytes = 1;

    while(bytes) {
        bytes = read(socketB, buffer, MAX_LENGTH);
        if(bytes <= 0)
            break;
        if (fwrite(buffer, bytes, 1, fd) < 0) 
            return -1;
    }

    fclose(fd);

    return read_response(socketA, buffer);
}
