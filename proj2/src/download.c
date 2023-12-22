#include "../include/download.h"

void print_url_info(const struct Url* url) {
    printf("User: %s\nPassword: %s\nFile: %s\n", url->user, url->password, url->file);
}

void handle_error(const char* message) {
    fprintf(stderr, "%s", message);
    exit(-1);
}

void download_FTP_file(const char* url_name) {
    struct Url url;
    memset(&url, 0, sizeof(url));

    if (parse_url(url_name, &url) != 0)
        handle_error("There was an error parsing the URL.\n");

    print_url_info(&url);

    char response[MAX_LENGTH];
    int control_socket = open_connection(url.ip, FTP_PORT);

    if (control_socket < 0 || read_response(control_socket, response) != SV_READY_FOR_NEW_USER)
        handle_error("Socket connection failed.\n");

    if (login(control_socket, url.user, url.password) != SV_LOGIN_SUCCESSFUL)
        handle_error("Login failed.\n");

    int port;
    char ip[MAX_LENGTH];
    if (passive_mode(control_socket, ip, &port) != SV_PASSIVE_MODE)
        handle_error("Passive mode failed.\n");

    int data_socket = open_connection(ip, port);
    if (data_socket < 0)
        handle_error("Secondary socket connection failed.\n");

    if (request_path(control_socket, url.path) != SV_READY_TRANSFER)
        handle_error("Path request failed.\n");

    if (get_path(control_socket, data_socket, url.file) != SV_TRANSFER_COMPLETE)
        handle_error("File transfer failed.\n");

    if (close_connection(control_socket, data_socket) != 0)
        handle_error("Error closing sockets.\n");

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
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
