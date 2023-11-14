// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE       0x00
#define FILE_NAME       0x01

#define OCTET_MULT      256
#define MAX_SIZE 1024

// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);

int controlPacket_rx(int fd, uint8_t ctrl, uint8_t* buf, size_t* file_size, char* filename);

int controlPacket_tx(int fd, uint8_t ctrl, const char* filename, size_t file_size);

int prepare_file(int fd, const char* filename);

#endif // _APPLICATION_LAYER_H_
