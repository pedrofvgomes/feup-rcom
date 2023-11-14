// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#define FLAG            0x7E
#define A_ER            0x03
#define A_RE            0x01
#define C_SET           0x03
#define C_UA            0x07
#define C_RR(n)         ((n == 0) ? 0x05 : 0x85)
#define C_REJ(n)        ((n == 0) ? 0x01 : 0x81)
#define C_DISC          0x0B
#define I_CONTROL(n)    ((n == 0) ? 0x00 : 0x40)
#define ESC             0x7D
#define STUFF_XOR       0x20
#define BAUDRATE 38400
#define DATA_SIZE           1024
#define STUFFED_DATA_SIZE   (DATA_SIZE * 2 + 2)

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct {
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;


typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} LinkLayerState;

extern struct data_holder_s {
    // +5 for other frame fields (address, control, bcc1, flag) 
    uint8_t buffer[STUFFED_DATA_SIZE + 5];
    size_t length;
} data_holder;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(int fd, const unsigned char *buf, int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(int fd, unsigned char *packet);

// Close previously opened connection.
// Return "1" on success or "-1" on error.
int llclose(int fd);

////////////////////////////////////////////////////////////////////////

size_t stuff_data(const uint8_t* data, size_t length, uint8_t bcc2, uint8_t* stuffed_data);

size_t destuff_data(const uint8_t* stuffed_data, size_t length, uint8_t* data, uint8_t* bcc2);

void build_supervision_frame(int fd, uint8_t address, uint8_t control);

int read_supervision_frame(int fd, uint8_t address, uint8_t control, uint8_t* rej_byte);

void build_information_frame(int fd, uint8_t address, uint8_t control, const uint8_t* packet, size_t packet_length);

int read_information_frame(int fd, uint8_t address, uint8_t control, uint8_t repeated_ctrl);

////////////////////////////////////////////////////////////////////////

int open_serial_port(char* serial_port);

int close_serial_port(int fd, LinkLayerRole role);


#endif // _LINK_LAYER_H_
