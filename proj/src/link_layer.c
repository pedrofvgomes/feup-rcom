// Link layer protocol implementation

#include "link_layer.h"

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source


int receptor = 1;
int transmitter = 0;

int timeout=0;
int retransmissions=0;

int alarm_fd = 0;
int alarm_counter = 0;

size_t length;
uint8_t buffer[STUFFED_SIZE + 5];

LinkLayerRole role;

struct timespec start, end;

void alarm_handler(int signo) {
    alarm_counter++;
    if (write(alarm_fd, buffer, length) != length) {
        return;
    }
    alarm(timeout);

    if (alarm_counter <= retransmissions)
        printf("Alarm%d\n", alarm_counter);
}

int llopen(LinkLayer connectionParameters) {
    
    int fd = open_serial_port(connectionParameters.serialPort);
    if (fd < 0) return -1;
    clock_gettime(CLOCK_REALTIME, &start);

    switch (connectionParameters.role)
    {
    case LlTx:
        role = LlTx;

        retransmissions = connectionParameters.nRetransmissions;
        timeout = connectionParameters.timeout;

        alarm_counter = 0;
        alarm_fd = fd;

        (void)signal(SIGALRM, alarm_handler);
        alarm(timeout);

        if (alarm_counter == 0) {
            create_supervision_frame(fd, A_ER, C_SET);

            if (write(fd, buffer, length) != length) {
                return -1;
            }
            alarm(timeout);
        }

        if (get_supervision_frame(fd, A_RE, C_UA, NULL) != 0) {
            if (alarm_counter == 0)
                alarm(0);
            return -1;
        }

        if (alarm_counter == 0) 
            alarm(0);  

        break;
    
    case LlRx:
        role = LlRx;

        while (get_supervision_frame(fd, A_ER, C_SET, NULL) != 0) {}
        create_supervision_frame(fd, A_RE, C_UA);    

        if (write(fd, buffer, length) != length) 
            return -1;
        break;

    default:
        return -1;
        break;
    }
    return fd;
}

int llwrite(int fd, const unsigned char *buf, int bufSize) {
    if (role == LlRx) 
        return -1;
    
    alarm_counter = 0;

    create_information_frame(fd, A_ER, I_CONTROL(transmitter), buf, bufSize);
     
    if (write(fd, buffer, length) != length) 
        return -1;
    
    alarm(timeout);

    int res = -1;
    uint8_t rej_ctrl = C_REJ(1 - transmitter);
    
    while (res != 0) {
        res = get_supervision_frame(fd, A_RE, C_RR(1 - transmitter), &rej_ctrl);
        if (res == 1) 
            // alarm > than retrannsmissions
            break;
        
    }
    alarm(0);
    if (res == 1)
        return -1;
    transmitter = 1 - transmitter;

    return bufSize;
}

int llread(int fd, unsigned char *packet) {
    if (role == LlTx)
        return -1;

    sleep(1);

    if (get_information_frame(fd, A_ER, I_CONTROL(1 - receptor), I_CONTROL(receptor)) != 0) {
        create_supervision_frame(fd, A_RE, C_RR(1 - receptor));
        if (write(fd, buffer, length) != length)
            return -1;
    }

    uint8_t data[DATA_SIZE];
    uint8_t bcc2;
    size_t data_size = destuffing(buffer, length, data, &bcc2);

    uint8_t tmp_bcc2 = 0;
    for (size_t i = 0; i < data_size; i++)
        tmp_bcc2 ^= data[i];

    if (tmp_bcc2 != bcc2) {
        create_supervision_frame(fd, A_RE, C_REJ(1 - receptor));
        if (write(fd, buffer, length) != length)
            return -1;

    }

    memcpy(packet, data, data_size);
    create_supervision_frame(fd, A_RE, C_RR(receptor));
    if (write(fd, buffer, length) != length)
        return -1;

    receptor = 1 - receptor;

    return data_size;
}


int llclose(int fd) {

    if (close_serial_port(fd, role)) return -1;

    if (tcdrain(fd) == -1) return -1;
    
    //if (tcsetattr(fd, TCSANOW, &oldtio) == -1) return -1; 
    close(fd);

    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = (end.tv_sec-start.tv_sec)+ (end.tv_nsec-start.tv_nsec)/1e9;
    printf("Elapsed Time: %f seconds\n",elapsed);

    return 1;
}

////////////////////////////////////////////

size_t stuffing(const uint8_t* data, size_t length, uint8_t bcc2, uint8_t* stuffed_data) {
    size_t stuffedSize = 0;

    for (int i = 0; i < length; i++) {
        if (data[i] == FLAG || data[i] == ESC) {
            stuffed_data[stuffedSize++] = ESC;
            stuffed_data[stuffedSize++] = data[i] ^ STUFF_XOR;
        } 
        else
            stuffed_data[stuffedSize++] = data[i];
    }

    if (bcc2 == FLAG || bcc2 == ESC) {
        stuffed_data[stuffedSize++] = ESC;
        stuffed_data[stuffedSize++] = bcc2 ^ STUFF_XOR;
    } 
    else
        stuffed_data[stuffedSize++] = bcc2;

    return stuffedSize;
}

size_t destuffing(const uint8_t* stuffed_data, size_t length, uint8_t* data, uint8_t* bcc2) {
    uint8_t destuffed_data[DATA_SIZE + 1];
    size_t aux = 0;

    for (size_t i = 0; i < length; i++) {
        if (stuffed_data[i] == ESC) {
            i++;
            destuffed_data[aux++] = stuffed_data[i] ^ STUFF_XOR;
        } 
        else
            destuffed_data[aux++] = stuffed_data[i];
    }

    *bcc2 = destuffed_data[aux - 1];
    memcpy(data, destuffed_data, aux - 1);

    return aux - 1;
}

void create_supervision_frame(int fd, uint8_t address, uint8_t control) {
    length = 5;
    memcpy(buffer, (uint8_t[]){FLAG, address, control, address ^ control, FLAG}, length);
}

int get_supervision_frame(int fd, uint8_t address, uint8_t control, uint8_t* rej_ctrl) {
    uint8_t byte;
    LinkLayerState state = START;

    uint8_t is_rej;
    while (state != STOP ) {
        if (alarm_counter > retransmissions) return 1;

        if (read(fd, &byte, 1) != 1) 
            continue;
        if (state == START) {
            if (byte == FLAG) 
                state = FLAG_RCV;
        } 
        else if (state == FLAG_RCV) {
            is_rej = 0;
            if (byte == address)
                state = A_RCV;
            else if (byte != FLAG) 
                state = START;
        } 
        else if (state == A_RCV) {
            if (rej_ctrl != NULL) {
                if (byte == *rej_ctrl)
                    is_rej = 1;
            }
            if (byte == control || is_rej)
                state = C_RCV;
            else if (byte == FLAG)
                state = FLAG_RCV;
            else
                state = START;
        } 
        else if (state == C_RCV) {
            if ((!is_rej && byte == (address ^ control)) || (is_rej && byte == (address ^ *rej_ctrl)))
                state = BCC_OK;
            else if (byte == FLAG)
                state = FLAG_RCV;
            else
                state = START;
        } 
        else if (state == BCC_OK) {
            if (byte == FLAG) {
                if (is_rej)
                    return 2;
                state = STOP;
            }
            else
                state = START;
        }
    }
    return 0;
}

void create_information_frame(int fd, uint8_t address, uint8_t control, const uint8_t* packet, size_t packet_length) {
    uint8_t bcc2 = 0;
    for (size_t i = 0; i < packet_length; i++)
        bcc2 ^= packet[i];

    uint8_t stuffed_data[STUFFED_SIZE];
    size_t stuffed_length = stuffing(packet, packet_length, bcc2, stuffed_data);

    memcpy(buffer + 4, stuffed_data, stuffed_length);
    buffer[0] = FLAG;
    buffer[1] = address;
    buffer[2] = control;
    buffer[3] = address ^ control;
    buffer[4 + stuffed_length] = FLAG;
    length = 4 + stuffed_length + 1;
}

int get_information_frame(int fd, uint8_t address, uint8_t control, uint8_t repeated_ctrl) {
    uint8_t byte;
    LinkLayerState state = START;

    uint8_t is_repeated;
    length = 0;
    memset(buffer, 0, STUFFED_SIZE + 5);

    while (state != STOP) {
        if (alarm_counter > retransmissions) 
            return 1;

        if (read(fd, &byte, 1) != 1) 
            continue;

        if (state == START) {
            if (byte == FLAG) 
                state = FLAG_RCV;
        } 
        else if (state == FLAG_RCV) {
            is_repeated = 0;
            if (byte == address)
                state = A_RCV;
            else if (byte != FLAG) 
                state = START;
        } 
        else if (state == A_RCV) {
            if (byte == repeated_ctrl) 
                is_repeated = 1;
            if (byte == control || is_repeated)
                state = C_RCV;
            else if (byte == FLAG)
                state = FLAG_RCV;
            else
                state = START;
        } 
        else if (state == C_RCV) {
            if ((!is_repeated && byte == (address ^ control)) || (is_repeated && byte == (address ^ repeated_ctrl)))
                state = BCC_OK;
            else if (byte == FLAG)
                state = FLAG_RCV;
            else
                state = START;
        } 
        else if (state == BCC_OK) {
            if (byte == FLAG) {
                state = STOP;
                if (is_repeated)
                    return 2;
            } else
                buffer[length++] = byte;
        }
    }

    return 0;
}

////////////////////////////////////////////

int open_serial_port(char* serial_port)
{
    int fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;

    struct termios oldtio;
    struct termios newtio;

    if (tcgetattr(fd, &oldtio) == -1) return -1;

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    tcflush(fd, TCIOFLUSH);

    //if (tcsetattr(fd, TCSANOW, &newtio) == -1) return -1;

    return fd;
}

int close_serial_port(int fd, LinkLayerRole role) {
    if(role == LlTx) {

        alarm_counter = 0;
        (void) signal(SIGALRM, alarm_handler);

        create_supervision_frame(fd, A_ER, C_DISC);
        if (write(fd, buffer, length) != length) return -1;

        alarm(timeout);

        int flag = 0;
        for (;;) {
            if (get_supervision_frame(fd, A_RE, C_DISC, NULL) == 0) {
                flag = 1;
                break;
            }

            if (alarm_counter == retransmissions) break;
        }
        alarm(0);

        if (!flag) return -1;

        create_supervision_frame(fd, A_ER, C_UA);
        if (write(fd, buffer, length) != length) return -1;

        return 0;
    }

    else if(role == LlRx) {
        while (get_supervision_frame(fd, A_ER, C_DISC, NULL) != 0) {}

        create_supervision_frame(fd, A_RE, C_DISC);
        if (write(fd, buffer, length) != length) return -1;

        while (get_supervision_frame(fd, A_ER, C_UA, NULL) != 0) {}

        return 0;
    }
    return -1;
}
