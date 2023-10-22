// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    // estabelecer a conexao
    LinkLayerState state=START;
    unsigned char byte_now;
    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(connectionParameters.serialPort);
        return -1; 
    }

    switch (connectionParameters.role)
    {
    // enviar SET e esperar pelo UA
    case LlRx:
        while(state != STOP)  {
            read(fd, &byte_now, 1);
            switch (state)
            {
            case START:
                if(byte_now == 0x7E) 
                    state=FLAG_RCV;
                break;

            case FLAG_RCV:
                if(byte_now == 0x03)
                    state=A_RCV;

                else if(byte_now != 0x7E)   
                    state=START; 
                break;    

             case A_RCV:
                if(byte_now == 0x03)
                    state= C_RCV;

                else if(byte_now == 0x7E) 
                    state = FLAG_RCV;

                else 
                    state=START;       
                break;    

            case C_RCV:
                if(byte_now == (0x03 ^ 0x03)) 
                    state = BCC_OK;

                else if(byte_now==0x7E)
                    state = FLAG_RCV;

                else
                    state=START;    
                break;

            case BCC_OK:
                if(byte_now == 0x7E) 
                    state = STOP;

                else 
                    state = START;    
                break;
            default:
                break;
            }
        }
        break;

    // esperar pelo SET e enviar UA
    case LlTx:

        break;

    default:
        return -1;
        break;
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
