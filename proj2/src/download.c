#include <stdio.h>

int main(int argc, char *argv[]) {
    // usage
    if(argc != 2) {
        fprintf(stderr, "Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    // parse 
    // criar socket a
    // autenticacao
    // passive mode
    // criar socket b
    // request resource
    // get resource
    // close connection
    return 0;
}