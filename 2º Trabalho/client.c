#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define BUFSIZE 256        // Tamanho máximo (mensagem)

char ERROR[]= "Erro.\n";
char message[]="MSG";
char nickname[]="NICK";
char user_info[]="INFO\n";
char add_tag[]="TAG";
char post[]="POST";
char read_post[]="READ";
char file[]="FILE";
char exit_chat[]="EXIT\n";

void chat(int sockfd, int maxfdp, fd_set rset){

    int n;
    char buffer[BUFSIZE];	    // array para a mensagem
    char aux[BUFSIZE];			// auxiliar que será comparado com o input do utilizador				

    while(1){

        FD_ZERO(&rset);
        FD_SET(0,&rset);		
        FD_SET(sockfd, &rset);	

        // Verificação de mudança de cliente
        if(select(maxfdp, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <0) {
            perror("select");
            exit(1);
        }

        /************** Imprimir mensagens de outros clientes ***************/

        // Verificar se há um socket em FD_SET
        if(FD_ISSET(sockfd,&rset))
        {
            memset(buffer, 0, sizeof(buffer));	// Inicializar buffer, array da mensagem
            
            // ler as mensagens do chat
            if((n = read(sockfd, buffer, sizeof(buffer))) > 0)
                write(1, buffer, n);  //escreve a mensagem na tela do cliente
        }

        /*************** Enviar a sua mensagem para o servidor ***************/

        if(FD_ISSET(0, &rset))
        {
            memset(aux, 0, sizeof(aux));

            if((n = read(0, aux, sizeof(aux))) > 0 )
            {     
                // NICK = adiciona o nickname ao cliente
                if(strstr(aux, nickname) != NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // INFO = mostra a informação dos utilizadores conectados (NICKNAME/TAG)
                if(!strcmp(aux,user_info))
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // TAG =  adiciona uma tag ao utilizador
                if(strstr(aux,add_tag)!=NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // MSG = troca de mensagens entre os utilizadores com uma determinada tag
                if(strstr(aux,message) != NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // POST = envio de posts para a tag GLOBAL
                if(strstr(aux,post) != NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // READ = leitura dos posts efetuados
                if(strstr(aux,read_post) != NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }

                // FILE = inicia o envio do ficheiro <filename>, cujo tamanho é de <bytes>, para a tag <tag>
                if (strstr(aux,file) != NULL)
                {
                    write(sockfd, aux, strlen(aux));
                    continue;
                }
                
                // EXIT = utilizador saiu do chat
                if(!strcmp(aux,exit_chat))
                {
                    write(sockfd, aux, strlen(aux));
                    break;
                }
            }
        }
    }
}

void main(int argc,char *argv[])
{
    int sockfd, maxfdp;						
    struct sockaddr_in servaddr;			
    fd_set rset;

    if(argc < 2){
        printf("usage:%s [port_number]\n",argv[0]);
        exit(0);
    }

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_family=AF_INET;	
    servaddr.sin_port=htons(atoi(argv[1]));	            //./client PORT

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("connect");
        exit(0);
    }

    maxfdp=sockfd + 1;
    chat(sockfd, maxfdp, rset);
    close(sockfd);
}