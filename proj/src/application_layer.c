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

        // control packet para inicio de ficheiro (c = 2)
        unsigned int cpSize;
        unsigned char* cpStart = controlPacket(2, fileSize, filename, &cpSize);

        // llwrite do packet
        if(llwrite(fd,cpStart, cpSize) != 0){
            perror("llwrite error in start packet, aborting");
            exit(-1);
        }

        // enviar data packets enquanto eles ainda restarem
        int remainingBytes = fileSize;

        while(remainingBytes >= 0){
            
        }

        // control packet para fim do ficheiro (c = 3)
        unsigned char* cpEnd = controlPacket(3, fileSize, filename, &cpSize);

        // llwrite do packet
        if(llwrite(fd, cpStart, cpSize) != 0){
            perror("llwrite error in end packet, aborting");
            exit(-1);
        }

        // terminar
        llclose(fd);

        break;


    case LlRx:
        FILE* file;
        if(fileExtension(filename) == 'txt')
            file = fopen(filename, "w+");
        else
            file = fopen(filename, "wb+");
        
        if (file == NULL){
            perror("fopen error, aborting");
            exit(-1);
        }

        unsigned char* packet = (unsigned char*) malloc (MAX_PAYLOAD_SIZE);

        // loop 
            // ler packet size com llread, deve dar return >= 0
            // se packet size for 0, break
            // se c for 3, break (fim de ficheiro)
            // se c nao for 3, malloc para o buffer, remover header, escrever dados no ficheiro e libertar buffer

        while(1){
            int packetSize;
            while(1){
                packetSize = llread(fd, packet);
                if (packetSize >= 0) break;
            }

            if(packetSize == 0) break;

            if(packet[0] != 3){
                unsigned char* buffer = (unsigned char*) malloc (packetSize);
                
                // remover header (4 bytes)
                memcpy(buffer, packet+4, packetSize-4);
                buffer += 4 + packetSize;

                fwrite(buffer, sizeof(unsigned char), packetSize-4, file);
                free(buffer);
            }
            
            else break;
        }

        break;
    
    default:
        exit(-1);
        break;
    }

    fclose(file);
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


unsigned char* controlPacket(unsigned int c, unsigned int fileSize, unsigned char* filename, unsigned int* cpSize){
    // cpSize vai ser 1 (c) + 1 (t1) + 1 (t2) + l1 + l2
    unsigned int L1 = sizeof(fileSize);
    unsigned int L2 = sizeof(filename);

    *cpSize = 3+L1+L2;
    unsigned char* cp = (unsigned char*)malloc(3+L1+L2);

    cp[0] = c;
    cp[1] = 0; // filesize
    cp[2] = L1;

    // V1, byte por byte
    for(unsigned int i = 0; i<L1; i++){
        cp[L1+2-i] = fileSize & 0xff;
        fileSize = fileSize >> 8;
    }

    cp[L1] = 1; // t2
    cp[L1+1] = L2; // l2
    memcpy(L1+2, filename, L2); // v2, escrito diretamente por memcpy
                                // com tamanho l2 previamente calculado

    return cp;
}
