#include "../include/connection.h"

int openConnection(char *address, int port) {

    int sockfd;
    struct sockaddr_in server_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);  
    server_addr.sin_port = htons(port); 
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Opening socket failed\n");
        exit(-1);
    }
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Opening connection failed.\n");
        close(sockfd);
        exit(-1);
    }
    
    return sockfd;
}

int login(const int socket, const char* user, const char* pass) {

    char userCommand[BUFFER_SIZE]; 
    char passCommand[BUFFER_SIZE]; 
    char answer[MAX_LENGTH];

    sprintf(userCommand, "user %s\n", user);
    sprintf(passCommand, "pass %s\n", pass);
    
    write(socket, userCommand, strlen(userCommand));
    if (readResponse(socket, answer) != SV_READY_FOR_PASSWORD) {
        printf("Unknown user '%s'. Abort.\n", user);
        exit(-1);
    }

    write(socket, passCommand, strlen(passCommand));
    return readResponse(socket, answer);
}

int passiveMode(const int socket, char *ip, int *port) {

    char answer[MAX_LENGTH];
    int ip1, ip2, ip3, ip4, port1, port2;
    write(socket, "pasv\n", 5);
    if (readResponse(socket, answer) != SV_PASSIVE_MODE) 
        return -1;

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


int closeConnection(const int socketA, const int socketB) {
    
    char answer[MAX_LENGTH];
    write(socketA, "Quit\n", 5);

    if(readResponse(socketA, answer) != SV_END) {
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
