#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#define PORT 1300

int main (int argc, char *argv[]){

    pid_t pid;

    int sockfd=socket(AF_INET, SOCK_STREAM, 0), server_fd, new_socket, valread;
    int opt = 1;
    struct sockaddr_in serv_addr;    // vai ter toda a informaçao necessaria para a ligação (endereço)
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024];
   
    

    if (sockfd < 0){
        perror("ERROR opening socket");
        exit(1);
    }//verificação da criação do socket

    server_fd=sockfd;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pid=fork();

        if (pid<0)
            exit(EXIT_FAILURE);
        

        if (pid==0)
        {
            while (1)
            {
                valread = read( new_socket , buffer, 1024);

                if (valread==0)
                    break;
                
                printf("%s",buffer );
                //send(new_socket , hello , strlen(hello) , 0 );

                
                send(new_socket , buffer , strlen(buffer) , 0 );
                //printf("Hello message sent\n");
            }
        }
        else
        {
            while (1)
            {
                valread = read( new_socket , buffer, 1024);

                if (valread==0)
                    break;
                
                printf("%s",buffer );
                //send(new_socket , hello , strlen(hello) , 0 );

                
                send(new_socket , buffer , strlen(buffer) , 0 );
                //printf("Hello message sent\n");
            }
        }
    
    }
    
    /*
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    //send(new_socket , hello , strlen(hello) , 0 );

    
    send(new_socket , buffer , strlen(buffer) , 0 );
    //printf("Hello message sent\n");

    */
    return 0;
}