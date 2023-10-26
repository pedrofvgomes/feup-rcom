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


    int fd = llopen(connectionParameters);
    if(fd<0){
        perror("llopen error, aborting");
        exit(-1);
    }

    FILE *file;

    // consoante a role, logica para enviar ou receber ficheiro
    switch (connectionParameters.role)
    {
    case LlTx:
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
        if(llwrite(fd,cpStart, cpSize) == -1){
            perror("llwrite error in start packet, aborting");
            exit(-1);
        }

        // enviar data packets enquanto eles ainda restarem
        int remainingBytes = fileSize;
        unsigned char* data = (unsigned char*) malloc (sizeof(unsigned char) * fileSize);
        fread(data, sizeof(unsigned char), fileSize, fd);

        // contador do frame atual
        unsigned char frameNumber = 0;

        // vamos enviar MAX_PAYLOAD_SIZE bytes de cada vez
        while(remainingBytes >= 0){
            // calcular numero de bytes a enviar
            // temos de alocar sempre 4 bytes para o header
            // logo se o payload - 4 for maior ou igual ao numero de bytes restantes
            // enviamos tudo

            // numero de bytes a enviar
            int bytesToSend;
            if (MAX_PAYLOAD_SIZE - 4 >= remainingBytes){
                bytesToSend = remainingBytes;
            }
            else{
                bytesToSend = MAX_PAYLOAD_SIZE - 4;
            }

            // alocar espaço para mais 4 bytes - header
            unsigned char* dataPacket = (unsigned char*) malloc (bytesToSend + 4);
            
            // c
            dataPacket[0] = 1;

            // frame number
            dataPacket[1] = frameNumber;

            // l2
            dataPacket[2] = (bytesToSend >> 8) & 0xff;
            
            // l1
            dataPacket[3] = bytesToSend & 0xff;

            // data
            memcpy(dataPacket+4, data, bytesToSend);


            // llwrite
            if (llwrite(fd, dataPacket, bytesToSend+4) == -1){
                perror("llwrite error in data packet, aborting");
                exit(-1);
            }

            // decrementar o numero de bytes restantes
            remainingBytes -= MAX_PAYLOAD_SIZE;
            // offset do pointer do ficheiro, para continuar a ler dados
            data += bytesToSend;
            // atualizar frame number
            frameNumber += 1;
            frameNumber %= 255;
        }

        // control packet para fim do ficheiro (c = 3)
        unsigned char* cpEnd = controlPacket(3, fileSize, filename, &cpSize);

        // llwrite do packet
        if(llwrite(fd, cpStart, cpSize) == -1){
            perror("llwrite error in end packet, aborting");
            exit(-1);
        }

        // terminar
        llclose(fd);

        break;


    case LlRx:
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
