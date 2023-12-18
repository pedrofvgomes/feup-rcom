#include "../include/connection.h"

int open_connection(char *address, int port) {

    int socket_fd;
    struct sockaddr_in server_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);  
    server_addr.sin_port = htons(port); 
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Opening socket failed\n");
        exit(-1);
    }
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Opening connection failed.\n");
        close(socket_fd);
        exit(-1);
    }
    
    return socket_fd;
}

int login(const int socket, const char* user, const char* password) {

    char userRequest[BUFFER_SIZE]; 
    char passwordRequest[BUFFER_SIZE]; 
    char answer[MAX_LENGTH];

    sprintf(userRequest, "user %s\n", user);
    sprintf(passwordRequest, "pass %s\n", password);
    
    write(socket, userRequest, strlen(userRequest));
    if (read_response(socket, answer) != SV_READY_FOR_PASSWORD) {
        printf("Unknown user '%s'.\n", user);
        exit(-1);
    }

    write(socket, passwordRequest, strlen(passwordRequest));
    return read_response(socket, answer);
}

int passive_mode(const int socket, char *ip, int *port) {

    char answer[MAX_LENGTH];
    int ip1, ip2, ip3, ip4; 
    int port1, port2;
    write(socket, "pasv\n", 5);
    if (read_response(socket, answer) != SV_PASSIVE_MODE) 
        return -1;

    sscanf(answer, "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    *port = port1 * BUFFER_SIZE + port2;
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    return SV_PASSIVE_MODE;
}

int read_response(const int socket, char* buffer) {

    char byte;
    int index = 0;
    int response_code;
    ResponseState state = START;
    memset(buffer, 0, MAX_LENGTH);

    while (state != END) {
        if (read(socket, &byte, 1) <= 0)
            break;

        switch (state) {
            case START:
                if (byte == ' ') 
                    state = SINGLE;
                else if (byte == '-') 
                    state = MULTIPLE;
                else if (byte == '\n') 
                    state = END;
                else 
                    buffer[index++] = byte;
                break;
            case SINGLE:
                if (byte == '\n') 
                    state = END;
                else 
                    buffer[index++] = byte;
                break;
            case MULTIPLE:
                if (byte == '\n') {
                    memset(buffer, 0, MAX_LENGTH);
                    state = START;
                    index = 0;
                }
                else 
                    buffer[index++] = byte;
                break;
            case END:
                break;
            default:
                break;
        }
    }

    sscanf(buffer, "%d", &response_code);
    return response_code;
}


int close_connection(const int socketA, const int socketB) {
    
    char answer[MAX_LENGTH];
    write(socketA, "Quit\n", 5);

    if(read_response(socketA, answer) != SV_END) {
        printf("Error in closing connection.\n");
        return -1;
    }   

    int closeA = close(socketA);
    int closeB = close(socketB);

    if (closeA || closeB) {
        printf("Error in closing sockets.\n");
        return -1;
    }
    return 0;
}
