#include "../include/utils.h"

int parse_url(const char *url_name, struct Url *url) {

    regex_t slash, at_sign;
    regcomp(&slash, "/", 0);
    regcomp(&at_sign, "@", 0);

    if (regexec(&slash, url_name, 0, NULL, 0)) 
        return -1;

    // Default user and password

    if (regexec(&at_sign, url_name, 0, NULL, 0) != 0) { 
        strcpy(url->user, DEFAULT_USER);
        strcpy(url->password, DEFAULT_PASSWORD);
        sscanf(url_name, "%*[^/]//%[^/]", url->domain);
    } 

   // Specific user and password

    else {    
        sscanf(url_name, "%*[^/]//%[^:/]", url->user);
        sscanf(url_name, "%*[^/]//%*[^:]:%[^@\n$]", url->password);
        sscanf(url_name, "%*[^/]//%*[^@]@%[^/]", url->domain);
    }

    sscanf(url_name, "%*[^/]//%*[^/]/%s", url->path);
    strcpy(url->file, strrchr(url_name, '/') + 1);

    struct hostent *host;
    if (strlen(url->domain) == 0) 
        return -1;

    if ((host = gethostbyname(url->domain)) == NULL) {
        printf("Error in the domain.\n");
        return -1;
    }

    strcpy(url->ip, inet_ntoa(*((struct in_addr *) host->h_addr)));

    return !(strlen(url->domain) && strlen(url->user) && strlen(url->password) && strlen(url->path) && strlen(url->file));
}

int send_pasv(const int socket, char *serverResponse) {
    write(socket, "pasv\n", 5);
    if (read_response(socket, serverResponse) != SV_PASSIVE_MODE) {
        return -1; 
    }
    return 0;
}

int get_ip_and_port(const char *server_response, char *ip, int *port) {
    int ip1, ip2, ip3, ip4;
    int port_msb, port_lsb;
    if (sscanf(server_response, "%*[^()](%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port_msb, &port_lsb) != 6) {
        return -1;
    }
    *port = port_msb * BUFFER_SIZE + port_lsb;
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    return 0; 
}

int send_request(const int socket, const char* request) {
    if (write(socket, request, strlen(request)) < 0)
        return -1;
    return 0;
}

int request_path(const int socket, char *path) {

    char file_request[MAX_LENGTH];
    sprintf(file_request, "retr %s\n", path);
    write(socket, file_request, strlen(file_request));

    char response[MAX_LENGTH];
    return read_response(socket, response);
}

int get_path(const int control_socket, const int data_socket, char *filename) {

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file. \n");
        return -1;
    }

    char file_content[MAX_LENGTH];
    int bytes;

    while ((bytes = read(data_socket, file_content, MAX_LENGTH)) > 0) {
        if (fwrite(file_content, bytes, 1, file) < 0) {
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    return read_response(control_socket, file_content);
}
