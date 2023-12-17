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
