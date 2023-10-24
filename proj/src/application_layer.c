// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer connectionParameters;

    strcpy(connectionParameters.serialPort, serialPort);

    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;
    
    if(strcmp(role, "rx"))
       connectionParameters.role = LlRx; 
    else if(strcmp(role, "tx"))
        connectionParameters.role = LlTx;

    int fd = llopen(connectionParameters);
    if(fd<0){
        perror("llopen error, aborting");
        exit(-1);
    }

    // consoante a role, logica para enviar ou receber ficheiro

}
