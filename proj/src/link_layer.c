// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source


#define FLAG 0x7E
#define ESCAPE 0x7D
#define A_ER 0x03
#define A_RE 0x01
#define C_SET 0x03
#define C_DI 0x0B
#define C_UA 0x07
int alarmCall = FALSE;

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
                if(byte_now == FLAG) 
                    state=FLAG_RCV;
                break;

            case FLAG_RCV:
                if(byte_now == A_ER)
                    state=A_RCV;

                else if(byte_now != FLAG)   
                    state=START; 
                break;    

             case A_RCV:
                if(byte_now == C_SET)
                    state= C_RCV;

                else if(byte_now == FLAG) 
                    state = FLAG_RCV;

                else 
                    state=START;       
                break;    

            case C_RCV:
                if(byte_now == (A_ER ^ C_SET)) 
                    state = BCC_OK;

                else if(byte_now==FLAG)
                    state = FLAG_RCV;

                else
                    state=START;    
                break;

            case BCC_OK:
                if(byte_now == FLAG) 
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
        
        while(connectionParameters.nRetransmissions != 0 && state != STOP) {
            unsigned char trama[5] = {FLAG_RCV, A_RCV, C_RCV, A_RCV ^ C_RCV, FLAG_RCV};
            write(fd, trama, 5);
            alarm(connectionParameters.timeout);
            alarmCall = FALSE;
            while (alarmCall == FALSE && state != STOP)
            {
                read(fd, &byte_now, 1);
                switch (state)
                {
                case START:
                    if(byte_now == FLAG)
                        state = FLAG_RCV;
                    break;

                case FLAG_RCV:
                    if(byte_now == A_ER)
                        state = A_RCV;
                    else if(byte_now == FLAG)  
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;

                              
                default:
                    break;
                }
            }
            

        }

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
