// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


    FILE *file;
    int fd = llopen(connectionParameters);
    if(fd<0){
        perror("llopen error, aborting");
        exit(-1);
    }


    // consoante a role, logica para enviar ou receber ficheiro
    switch (connectionParameters.role)
    {
    case LlTx:
        FILE* file;
        if(fileExtension(filename) == 'txt')
            file = fopen(filename, "r");
        else
            file = fopen(filename, "rb");
        
        if (file == NULL){
            perror("fopen error, aborting");
            exit(-1);
        }

        // ir ao final do ficheiro, guardar a posicao (final = tamanho) e voltar ao inicio
        fseek(file, 0, SEEK_END);
        int fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        


        break;


    case LlRx:
        break;
    
    default:
        exit(-1);
        break;
    }

}


char* fileExtension(char* filename){
    int position;
    for(int i=0; i<strlen(filename); i++)
        if (filename[i] == '.')
            position = i;
    
    int len = strlen(filename) - 1 - position;

    char* extension = (char*) malloc(len*sizeof(char));
    int counter = 0;
    for(int i=position+1; i<strlen(filename); i++){
        extension[counter] = filename[i];
        counter++;
    }
    extension[counter] = '\0';
    
    return extension;
}