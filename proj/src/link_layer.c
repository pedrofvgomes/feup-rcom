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

struct termios oldtio;

int alarmCall = FALSE;
int accepted = 0;
int bccerror = 0;
unsigned char recieverFrameNumber = 0;

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
        unsigned char trama[5] = {FLAG_RCV, A_RE, C_UA, A_RE ^ C_UA, FLAG_RCV};
        write(fd, trama, 5);
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
                    else if(byte_now != FLAG)  
                        state = START;
                    break;

                case A_RCV:
                    if(byte_now == C_SET)
                        state = C_RCV;
                    else if(byte_now == FLAG)
                        state = FLAG_RCV;
                    else 
                        state = START;
                    break;   

                case C_RCV:
                    if(byte_now == (A_ER^C_SET))         
                        state = BCC_OK;
                    else if(byte_now == FLAG)   
                        state = FLAG_RCV;
                    else
                        state = START;            
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
        }
        connectionParameters.nRetransmissions--;
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
int llread(int fd, unsigned char *packet)
{
    LinkLayerState state = START;
    unsigned char control, current_byte;
    int i = 0;

    while(state != STOP){
        if(read(fd, &current_byte, 1) > 0){
            switch (state)
            {
            // state machine
            case START:
                if (current_byte == FLAG) 
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (current_byte == A_ER)
                    state = A_RCV;
                else if (current_byte != FLAG)
                    state = START;
                break;
            case A_RCV:
                if(current_byte == (recieverFrameNumber << 6)){
                    state = C_RCV;
                    control = current_byte;
                }
                else if (current_byte == (((recieverFrameNumber + 1)%2) << 6)){
                    // mandar frame a avisar que aceitamos, escrever para o ficheiro e return 0 para continuar a escrever
                    unsigned char frame[5] = {FLAG, A_RE, (((recieverFrameNumber + 1)%2) << 7) | 0x05, ((((recieverFrameNumber + 1)%2) << 7) | 0x05) ^ A_RE, FLAG};
                    write(fd, frame, 5);
                    return 0;
                }
                else if (current_byte == FLAG){
                    state = FLAG_RCV;
                }
                else if (current_byte == C_DI){
                    // mandar frame para disconectar
                    unsigned char frame[5] = {FLAG, A_RE, C_DI, A_RE ^ C_DI, FLAG};
                    write(fd, frame, 5);
                    if (tcsetaddr(fd, TCSANOW, &oldtio) == -1){
                        perror('tcsetaddr error, aborting');
                        exit(-1);
                    }
                    return close(fd);
                }
                else
                    state = START;
                break;
            case C_RCV:
                if (current_byte == (A_ER ^ control))
                    state = DATA;
                else if (current_byte == FLAG)
                    state = FLAG_RCV;
                else 
                    state = START;
                break;


            // data
            case DATA:
                if (current_byte == ESCAPE) 
                    state = STUFFED_BYTES;
                else if (current_byte == FLAG){
                    i--;
                    packet[i] = '\0';
                    unsigned char bcc2 = packet[i], test = packet[0];

                    for(unsigned int x = 1; x < i; x++){
                        test ^= packet[x];
                    }

                    if (bcc2 == test){
                        if (!accepted && bccerror){
                            // mandar frame a rejeitar
                            unsigned char frame[5] = {FLAG, A_RE, (((recieverFrameNumber + 1)%2) << 7) | 0x1, ((((recieverFrameNumber + 1)%2) << 7) | 0x1) ^ A_RE, FLAG};
                            write(fd, frame, 5);
                            accepted = 1;
                            return -1;
                        }

                        state = STOP;
                        // mandar frame a aceitar
                        unsigned char frame[5] = {FLAG, A_RE, (((recieverFrameNumber + 1)%2) << 7) | 0x5, ((((recieverFrameNumber + 1)%2) << 7) | 0x5) ^ A_RE, FLAG};
                        write(fd, frame, 5);
                        recieverFrameNumber += 1;
                        recieverFrameNumber %= 2;
                        accepted = 0;
                        return i;
                    }

                    else {
                        // mandar frame a rejeitar
                        unsigned char frame[5] = {FLAG, A_RE, (((recieverFrameNumber + 1)%2) << 7) | 0x1, ((((recieverFrameNumber + 1)%2) << 7) | 0x1) ^ A_RE, FLAG};
                        write(fd, frame, 5);
                        return -1;
                    }
                }
                else{
                    packet[i] = current_byte;
                    i++;
                }              
                break;
            case STUFFED_BYTES:
                state = DATA;
                packet[i] = current_byte ^ 0x20;
                i++;
                break;
            default:
                break;
            }
        }
    }

    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
