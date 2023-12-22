#ifndef UTILS_H
#define UTILS_H

#include "download.h"

#define MAX_LENGTH  500

struct Url {
    char user[MAX_LENGTH];      
    char password[MAX_LENGTH]; 
    char ip[MAX_LENGTH]; 
    char domain[MAX_LENGTH];             
    char path[MAX_LENGTH]; 
    char file[MAX_LENGTH];      
};

int parse_url(const char *url_name, struct Url *url);

int send_pasv(const int socket, char *serverResponse);

int get_ip_and_port(const char *server_response, char *ip, int *port);

int send_request(const int socket, const char* request);

int request_path(const int socket, char *resource);

int get_path(const int control_socket, const int data_socket, char *filename);

#endif /* UTILS_H */
