#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>

#include <string.h>

int main(int argc, char *argv[]) {

    int sockfd, portno, n;       // declarar o socket, porta, número de bytes que foram lidos
    struct sockaddr_in serv_addr;    // vai ter toda a informaçao necessaria para a ligação (endereço)
    struct hostent *server;      // e a estrutura retornada pelo gethostbyname

    char buffer[256];
    char buffer_msg[1024];
    portno = 1300;

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }    //verificação da criação do socket

    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }    // verificaçao de erros

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);      // especificar a porta 

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(2);
    }


    while (1)
    {

            bzero(buffer_msg,1024);
            fgets(buffer_msg,1024,stdin); 
            //printf("%s",buffer_msg); 
            
            if (strcmp(buffer_msg,"quit\n")==0){
                close(sockfd);
                return 0;
            }
            
            send(sockfd,buffer_msg,1024,0);

            // Now read server response 
            bzero(buffer,256);
            n = read(sockfd, buffer, 255);

            if (n < 0) {
                perror("ERROR reading from socket");
                exit(3);
            }

            printf("%s\n",buffer);
    }
        
    /* O código abaixo serve para receber apenas uma mensagem*/

    //close(sockfd);

    /*
        bzero(buffer_msg,256);
        fgets(buffer_msg,256,stdin);     
        send(sockfd,buffer_msg,256,0);

    // Now read server response 
    bzero(buffer,256);
    n = read(sockfd, buffer, 255);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(3);
    }

    printf("%s\n",buffer);
    

    close(sockfd);
        */
    //return 0;
}