#include "../include/download.h"

struct timespec start, end;

void handleError(const char* message) {
    printf("%s", message);
    exit(-1);
}

void printUrlInfo(const struct Url* url) {
    printf("Host: %s\nResource: %s\nFile: %s\nUser: %s\nPassword: %s\nIP Address: %s\n",
           url->host, url->resource, url->file, url->user, url->password, url->ip);
}

void downloadFTPFile(const char* urlPath) {
    struct Url url;
    memset(&url, 0, sizeof(url));

    if (parse(urlPath, &url) != 0) {
        handleError("There was an error parsing the URL.\n");
    }

    printUrlInfo(&url);

    char answer[MAX_LENGTH];
    int socketA = openConnection(url.ip, FTP_PORT);
    if (socketA < 0 || readResponse(socketA, answer) != SV_READY_FOR_NEW_USER) {
        handleError("Socket connection failed or not ready for user.\n");
    }

    if (login(socketA, url.user, url.password) != SV_LOGIN_SUCCESSFUL) {
        handleError("Authentication failed.\n");
    }

    int port;
    char ip[MAX_LENGTH];
    if (passiveMode(socketA, ip, &port) != SV_PASSIVE_MODE) {
        handleError("Passive mode failed.\n");
    }

    int socketB = openConnection(ip, port);
    if (socketB < 0) {
        handleError("Secondary socket connection failed.\n");
    }

    if (requestResource(socketA, url.resource) != SV_READY_TRANSFER) {
        handleError("Resource request failed.\n");
    }

    if (getResource(socketA, socketB, url.file) != SV_TRANSFER_COMPLETE) {
        handleError("File transfer failed.\n");
    }

    if (closeConnection(socketA, socketB) != 0) {
        handleError("Socket closure error.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        handleError(FTP_USAGE);
    }
    clock_gettime(CLOCK_REALTIME, &start);
    downloadFTPFile(argv[1]);
    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = (end.tv_sec-start.tv_sec)+ (end.tv_nsec-start.tv_nsec)/1e9;
    printf("Elapsed Time: %f seconds\n",elapsed);

    return 0;
}
