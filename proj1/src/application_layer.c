// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    LinkLayer connectionParameters;

    strcpy(connectionParameters.serialPort, serialPort);
    
    if(strcmp(role, "rx")==0)
        connectionParameters.role = LlRx;
    else if(strcmp(role, "tx")==0)
        connectionParameters.role = LlTx;  

    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    int fd = llopen(connectionParameters);
    if(fd<0) {
        fprintf(stderr, "llopen error, aborting\n");
        return;
    }

    printf("Connection successful\n");

    FILE* file;

    // consoante a role, logica para enviar ou receber ficheiro
    switch (connectionParameters.role)
    {
    // Enviar    
    case LlTx:
        if ((file = fopen(filename, "rb")) == NULL) {
            fprintf(stderr, "Error\n");
            return;
        }
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        if(controlPacket_tx(fd, 0x02, filename, fileSize) == -1) {
            fprintf(stderr, "Error\n");
            return;
        }

        uint8_t buf[MAX_SIZE - 3];
        size_t remainingBytes;

        while ((remainingBytes = fread(buf, 1, MAX_SIZE - 3, file)) > 0) {
            size_t dataPacket_length = 3 + remainingBytes;
            uint8_t dataPacket[dataPacket_length];

            dataPacket[0] = 0x01;
            dataPacket[1] = (uint8_t) (remainingBytes / OCTET_MULT);
            dataPacket[2] = (uint8_t) (remainingBytes % OCTET_MULT);

            memcpy(dataPacket + 3, buf, remainingBytes);

            if (llwrite(fd, dataPacket, dataPacket_length) == -1) {
                fprintf(stderr, "Error\n");
                return;
            }
        }
        if (controlPacket_tx(fd, 0x03, filename, fileSize) == -1) {
            fprintf(stderr, "Error\n");
            return;
        }
        fclose(file);
        printf("File sent\n");
        break;

    // Receber
    case LlRx:

        if (prepare_file(fd, filename) == -1) {
            fprintf(stderr, "Failed to receive file\n");
            return;
        }
        break;

    default:
        printf("Error\n");
        return;
        break;
    }

    if (llclose(fd) == -1) {
        fprintf(stderr, "Failed to close connection\n");
        return;
    }
    printf("Connection closed\n");
}

int controlPacket_tx(int fd, uint8_t ctrl, const char* filename, size_t file_size) {
    
    size_t file_length = strlen(filename) + 1;

    size_t dataPacket_length = 5 + file_length + sizeof(size_t);
    uint8_t dataPacket[dataPacket_length];

    dataPacket[0] = ctrl;
    dataPacket[1] = FILE_SIZE;
    dataPacket[2] = (uint8_t) sizeof(size_t);

    memcpy(dataPacket + 3, &file_size, sizeof(size_t));

    dataPacket[3 + sizeof(size_t)] = FILE_NAME;
    dataPacket[4 + sizeof(size_t)] = (uint8_t) file_length;
    memcpy(dataPacket + 5 + sizeof(size_t), filename, file_length);

    if (llwrite(fd, dataPacket, dataPacket_length) == -1) {
        fprintf(stderr, "llwrite error\n");
        return -1;
    }
    return 0;
}

int controlPacket_rx(int fd, uint8_t ctrl, uint8_t* buf, size_t* file_size, char* filename) {
    if (file_size == NULL) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    int size;
    if ((size = llread(fd, buf)) < 0) {
        fprintf(stderr, "llread error\n");
        return -1;
    }

    if (buf[0] != ctrl) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    uint8_t type;
    size_t length;
    size_t offset = 1;

    while (offset < size) {
        type = buf[offset++];
        if (type != FILE_SIZE && type != FILE_NAME) {
            fprintf(stderr, "Error\n");
            return -1;
        }

        if (type == FILE_SIZE) {
            length = buf[offset++];
            if (length != sizeof(size_t)) {
                fprintf(stderr, "Error\n");
                return -1;
            }
            memcpy(file_size, buf + offset, sizeof(size_t));
            offset += sizeof(size_t);
        } else {
            length = buf[offset++];
            if (length > MAX_SIZE - offset) {
                fprintf(stderr, "Error\n");
                return -1;
            }
            memcpy(filename, buf + offset, length);
            offset += length;
        }
    }

    return 0;
}

int prepare_file(int fd, const char* filename) {
    uint8_t buf[MAX_SIZE];
    size_t file_size;

    char received_filename[0xff];

    if (controlPacket_rx(fd, 0x02, buf, &file_size, received_filename) == -1) {
        fprintf(stderr, "Failure\n");
        return -1;
    }

    FILE* file;
    if ((file = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "Error opening file\n");
        return -1;
    }

    int size;
    while ((size = llread(fd, buf)) > 0) {
        if (buf[0] == 0x03) {
            break;
        }

        if (buf[0] != 0x01) {
            fprintf(stderr, "Error\n");
            return -1;
        }

        size_t length = buf[1] * OCTET_MULT + buf[2];
        uint8_t* data = (uint8_t*)malloc(length);
        memcpy(data, buf + 3, size - 3);

        if (fwrite(data, sizeof(uint8_t), length, file) != length) {
            fprintf(stderr, "Failure\n");
            return -1;
        }

        free(data);
    }

    fclose(file);
    return 0;
}
