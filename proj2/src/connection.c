#include "../include/connection.h"

int open_connection(char *address, int port) {

    int socket_fd;
    struct sockaddr_in server_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);  
    server_addr.sin_port = htons(port); 
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error opening socket.\n");
        return -1;
    }
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        close(socket_fd);
        return -1;
    }
    
    return socket_fd;
}

int login(const int socket, const char* user, const char* password) {

    char server_response[MAX_LENGTH];
    char user_request[BUFFER_SIZE]; 
    char password_request[BUFFER_SIZE]; 

    sprintf(user_request, "user %s\n", user);
    sprintf(password_request, "pass %s\n", password);
    
    if (send_request(socket, user_request) == -1) {
        printf("Error sending user request.\n");
        return -1;
    }

    if (read_response(socket, server_response) != SV_READY_FOR_PASSWORD) {
        printf("Unknown user. \n");
        return -1;
    }

    if (send_request(socket, password_request) == -1) {
        printf("Error sending password request.\n");
        return -1;
    }

    return read_response(socket, server_response);
}

int passive_mode(const int socket, char *ip, int *port) {

    char server_response[MAX_LENGTH];
    if (send_pasv(socket, server_response) != 0)
        return -1; 

    if (get_ip_and_port(server_response, ip, port) != 0)
        return -1; 

    return SV_PASSIVE_MODE;
}

int read_response(const int socket, char* buffer) {

    char byte;
    int index = 0;
    int response = -1;
    ResponseState state = START;
    memset(buffer, 0, MAX_LENGTH);

    while (state != END) {
        if (read(socket, &byte, 1) <= 0 || index >= MAX_LENGTH - 1)
            break;

        if (state == START) {
            if (byte == ' ')
                state = SINGLE;
            else if (byte == '-')
                state = MULTIPLE;
            else if (byte == '\n')
                state = END;
            else
                buffer[index++] = byte;
        } 
        else if (state == SINGLE) {
            if (byte == '\n')
                state = END;
            else
                buffer[index++] = byte;
        } 
        else if (state == MULTIPLE) {
            if (byte == '\n') {
                memset(buffer, 0, MAX_LENGTH);
                state = START;
                index = 0;
            } 
            else
                buffer[index++] = byte;
        }
    }

    if (sscanf(buffer, "%d", &response) != 1)
        response = -1;
    return response;
}


int close_connection(const int control_socket, const int data_socket) {
    
    char response[MAX_LENGTH];
    write(control_socket, "quit\n", 5);

    if(read_response(control_socket, response) != SV_END) {
        printf("Error in closing connection.\n");
        return -1;
    }   

    int close_control = close(control_socket);
    int close_data = close(data_socket);

    if (close_control || close_data) {
        printf("Error in closing sockets.\n");
        return -1;
    }
    return 0;
}
