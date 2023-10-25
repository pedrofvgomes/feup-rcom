// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source


#define FLAG 0x7E
#define ESCAPE 0x7D
#define A_ER 0x03
#define A_RE 0x01
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07

#define C_RR(Nr) ((Nr << 7) | 0x05)
#define C_REJ(Nr) ((Nr << 7) | 0x01)
#define C_N(Ns) (Ns << 6)

int alarmCall = FALSE;
int alarmTriggered = FALSE;
int retransmitions =0;


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
            byte_now = read(fd, &byte_now, 1);
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
        unsigned char trama[5] = {FLAG, A_RE, C_UA, A_RE ^ C_UA, FLAG};
        write(fd, trama, 5);
        break;

    // esperar pelo SET e enviar UA
    case LlTx:
        
        while(connectionParameters.nRetransmissions != 0 && state != STOP) {
            unsigned char trama[5] = {FLAG, A_RE, C_UA, A_RE ^ C_UA, FLAG};
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

    return fd;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(int fd, const unsigned char *buf, int bufSize)
{

    if (fd < 0) {
        perror(connectionParameters.serialPort);
        return -1; 
    }
    int tx=0;
    int tramaSize = bufSize+6;
    unsigned char *trama = (unsigned char *) malloc(tramaSize);
    trama[0] = FLAG;
    trama[1] = A_ER;
    trama[2] = C_N(tx);
    trama[3] = trama[1] ^trama[2];

    int aux = 4;
    for(int i=0; i<bufSize; i++ ) {
         if(buf[i] == FLAG || buf[i] == ESCAPE) {
            trama = realloc(trama,++tramaSize);
            trama[aux] = ESCAPE;
            aux++;
        }
        trama[aux] = buf[i];
        aux++;
    }
    trama[aux] = BCC_OK;
    aux++;
    trama[aux] = FLAG;  

    int currentTransmition = 0;
    int rejected = 0, accepted = 0;

    while (currentTransmition < retransmitions) { 
        alarmTriggered = FALSE;
        alarm(timeout);
        rejected = 0;
        accepted = 0;
        while (alarmTriggered == FALSE && !rejected && !accepted) {

            write(fd, trama, aux);
            unsigned char result = 0, byte_now = 0;
            LinkLayerState state = START;
            while(state != STOP)  {
                byte_now = read(fd, &byte_now, 1);
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
                        if(byte == C_RR(0) || byte == C_RR(1) || byte == C_REJ(0) || byte == C_REJ(1) || byte == C_DISC) {
                            state= C_RCV;
                            result = byte_now;
                        }
                        else if(byte_now == FLAG) 
                            state = FLAG_RCV;
                        else 
                            state=START;       
                    break;    

                    case C_RCV:
                        if(byte_now == (A_ER ^ result)) 
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
         
            if(!result)
                continue;
            else if(result == C_REJ(0) || result == C_REJ(1))
                rejected = 1;
            else if(result == C_RR(0) || result == C_RR(1)) {
                accepted = 1;
                tx = (tx+1) % 2;
            }
            else 
                continue;

        }
        if (accepted) break;
        currentTransmition++;
    }

    free(trama);
    if(accepted)
        return tramaSize;
    else
        llclose(fd);
    return -1;        
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(int fd, unsigned char *packet)
{
    unsigned char byte;
    LinkLayerState state = START;

    /*int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(connectionParameters.serialPort);
        return -1; 
    }*/

    while (state != STOP) 
    {
        byte = read(fd, &byte_now, 1);
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

            // falta a parte de ler o packet
                break;

        }
    }
    

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
