#include "../include/download.h"

void handle_error(const char* message) {
    fprintf(stderr, "%s", message);
    exit(-1);
}

void print_url_info(const struct Url* url) {
    printf("Host: %s\nResource: %s\nFile: %s\nUser: %s\nPassword: %s\nIP Address: %s\n",
           url->host, url->resource, url->file, url->user, url->password, url->ip);
}

void download_FTP_file(const char* url_path) {
    struct Url url;
    memset(&url, 0, sizeof(url));

    if (parse_url(url_path, &url) != 0)
        handle_error("There was an error parsing the URL.\n");

    print_url_info(&url);

    char answer[MAX_LENGTH];
    int socketA = open_connection(url.ip, FTP_PORT);

    if (socketA < 0 || read_response(socketA, answer) != SV_READY_FOR_NEW_USER)
        handle_error("Socket connection failed or not ready for user.\n");

    if (login(socketA, url.user, url.password) != SV_LOGIN_SUCCESSFUL)
        handle_error("Authentication failed.\n");

    int port;
    char ip[MAX_LENGTH];
    if (passive_mode(socketA, ip, &port) != SV_PASSIVE_MODE)
        handle_error("Passive mode failed.\n");

    int socketB = open_connection(ip, port);
    if (socketB < 0)
        handle_error("Secondary socket connection failed.\n");

    if (request_resource(socketA, url.resource) != SV_READY_TRANSFER)
        handle_error("Resource request failed.\n");

    if (get_resource(socketA, socketB, url.file) != SV_TRANSFER_COMPLETE)
        handle_error("File transfer failed.\n");

    if (close_connection(socketA, socketB) != 0)
        handle_error("Socket closure error.\n");

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        handle_error(FTP_USAGE);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    
    download_FTP_file(argv[1]);

    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = (end.tv_sec-start.tv_sec)+ (end.tv_nsec-start.tv_nsec)/1e9;
    printf("Elapsed Time: %f seconds\n",elapsed);

    return 0;
}
