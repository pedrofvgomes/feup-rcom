// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL_DATA    0x01
#define CONTROL_START   0x02
#define CONTROL_END     0x03

#define FILE_SIZE       0x00
#define FILE_NAME       0x01

#define OCTET_MULT      256
#define MAX_PACKET_SIZE 1024


int controlPacket(uint8_t control, const char* filename, size_t file_size) {
    size_t filename_length = strlen(filename) + 1;
    if (filename_length > 0xff) {
        fprintf(stderr, "Filename too long\n");
        return -1;
    }

    size_t packet_length = 5 + filename_length + sizeof(size_t);
    uint8_t packet[packet_length];

    packet[0] = control;

    packet[1] = FILE_SIZE;
    packet[2] = (uint8_t) sizeof(size_t);
    memcpy(packet + 3, &file_size, sizeof(size_t));

    packet[3 + sizeof(size_t)] = FILE_NAME;
    packet[4 + sizeof(size_t)] = (uint8_t) filename_length;
    memcpy(packet + 5 + sizeof(size_t), filename, filename_length);

    if (llwrite(packet, packet_length) == -1) {
        fprintf(stderr, "Error\n");
        return -1;
    }
    return 0;
}

int send_data_packet(const uint8_t* data, size_t length) {
    size_t packet_length = 3 + length;
    uint8_t packet[packet_length];

    packet[0] = CONTROL_DATA;
    packet[1] = (uint8_t) (length / OCTET_MULT);
    packet[2] = (uint8_t) (length % OCTET_MULT);
    memcpy(packet + 3, data, length);

    if (llwrite(packet, packet_length) == -1) {
        fprintf(stderr, "Error\n");
        return -1;
    }
    return 0;
}


int read_control_packet(uint8_t control, uint8_t* buf, size_t* file_size, char* received_filename) {
    if (file_size == NULL) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    int size;
    if ((size = llread(buf)) < 0) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    if (buf[0] != control) {
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
            if (length > MAX_PACKET_SIZE - offset) {
                fprintf(stderr, "Error\n");
                return -1;
            }
            memcpy(received_filename, buf + offset, length);
            offset += length;
        }
    }

    return 0;
}

int receive_file(const char* filename) {
    uint8_t buf[MAX_PACKET_SIZE];
    size_t file_size;

    char received_filename[0xff];

    if (read_control_packet(CONTROL_START, buf, &file_size, received_filename) == -1) {
        fprintf(stderr, "Failure\n");
        return -1;
    }

    FILE* file;
    if ((file = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "Error opening file\n");
        return -1;
    }

    int size;
    while ((size = llread(buf)) > 0) {
        if (buf[0] == CONTROL_END) {
            break;
        }

        if (buf[0] != CONTROL_DATA) {
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

        if(controlPacket(CONTROL_START, filename, fileSize) == -1) {
            fprintf(stderr, "Error\n");
            return;
        }

        uint8_t buf[MAX_PACKET_SIZE - 3];
        size_t bytes_read;

        while ((bytes_read = fread(buf, 1, MAX_PACKET_SIZE - 3, file)) > 0) {
            if (send_data_packet(buf, bytes_read) == -1) {
                fprintf(stderr, "Error\n");
                return;
            }
        }
        if (controlPacket(CONTROL_END, filename, fileSize) == -1) {
            fprintf(stderr, "Error\n");
            return;
        }
        fclose(file);
        printf("File sent\n");
        break;

    // Receber
    case LlRx:

        if (receive_file(filename) == -1) {
            fprintf(stderr, "Failed to receive file\n");
            return;
        }
        break;

    default:
        printf("Error\n");
        return;
        break;
    }

    if (llclose(0) == -1) {
        fprintf(stderr, "Failed to close connection\n");
        return;
    }
    printf("Connection closed\n");
}
