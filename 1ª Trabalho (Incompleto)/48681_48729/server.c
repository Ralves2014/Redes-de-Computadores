#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
   
#define PORT 1234
#define BUFSIZE 256
#define NAMESIZE 15
#define MAXC 30
#define NEW_CLIENT 0
#define CONNECTED 1

int clients[MAXC];
char usernames[MAXC][NAMESIZE];

int process_client(int sock, int fdmax, int server_fd)          
{
    int n;
    
    char buf[BUFSIZE];
    n = read(sock, buf, BUFSIZE);

    if (n <= 0){
        return 0; /* client closed socket */
    }
        
        
    buf[n] = '\0';

    if (buf[0]=='+')
    {
        char mensage[BUFSIZE];
        for (int c = 1; c < strlen(buf); c++)
            mensage[c-1]=buf[c];
            
        for (int i = 2; i <= fdmax; i++)
        {
            if (i!=server_fd)
                write(i, mensage, strlen(mensage));
                    
        }
        memset(mensage,0,sizeof(mensage));
    }
    else if (buf[0]=='-')
    {
        char nickname[NAMESIZE];
        char mensage[BUFSIZE];
        int c=1;

        while (buf[c]!=' ')
        {
            nickname[c-1]=buf[c];
            c++;
        }

        nickname[c]='\0';
                
        for (int c2 = c+1; c2 < strlen(buf); c2++)
            mensage[c2-c-1]=buf[c2];
                
            
        for (int i = 0; i < MAXC; i++)
        {
            //printf("%s", usernames[i]);
            fprintf(stderr, "username: %s; nick %s\n", usernames[i], nickname);
            if (strcmp(usernames[i],nickname)==0)
                    write(i+4,mensage,strlen(mensage));
                    
        }
        memset(nickname,0,sizeof(nickname));
        memset(mensage,0,sizeof(mensage));
    }

    return 1;
}

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket; 
    struct sockaddr_in address;
    
    int opt = 1;      // for setsockopt() SO_REUSEADDR, below
    int addrlen = sizeof(address);
    int i;      //n
    
    char buffer[BUFSIZE];

    fd_set master, read_fds;
    int fdmax;
    
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 1300 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Bind the socket to the network address and port
    if (  bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0  ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }

    for (int client = 0; client < MAXC; client++){
        clients[client]=0;
        strcat(usernames[client],"");
    }
        
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &master);

    fdmax = server_fd;
    
    // Main loop
    while (1) {
        read_fds = master;

        select(fdmax+1, &read_fds, NULL, NULL, NULL);

        for (i = 0; i <= fdmax; i++) {

            if (FD_ISSET(i, &read_fds)) {
                if (i == server_fd) { // New conection, accept() it

                    if ((new_socket = accept(server_fd,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
                        perror("accept failed"); 
                        exit(EXIT_FAILURE); 
                    }
                    int n;
                    //printf("I:%d   new:%d\n", i, new_socket);
                    char name_client[NAMESIZE];
                    n=read(new_socket,name_client,NAMESIZE);  
                    //fprintf(stderr,"n:%d", n);
                    clients[new_socket-4]=CONNECTED;
                    strcpy(usernames[new_socket-4],name_client);
                    
                    // teste
                    /*
                    for (int k = 0; k < MAXC; k++)
                    {
                        fprintf(stderr, "%d: %s;", k, usernames[k]);
                    }
                    */
                    
                    printf("Client %s", usernames[new_socket-4]);
                    
                    if (new_socket > fdmax)
                        fdmax = new_socket;
                    
                    FD_SET(new_socket, &master);
                    memset(name_client,0,sizeof(name_client));
                    
                }
                else { // "Old" client sent data, read() it
                    if (process_client(i,fdmax,server_fd) == 0) { // client close()d

                        FD_CLR(i, &master);
                        close(i);
                        printf("Client disconnected.\n");      
                    }
                    else { /* already processed */ }
                }
            }
        }
    }
    return 0; 
} 
