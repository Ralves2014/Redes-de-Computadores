#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>

#define MAX_CLIENT 10
#define MAXLINE 256
#define CHATDATA 256
#define NICKNAME 10
#define INVALID_SOCK -1
#define TAGSIZE 25
#define NUM_TAG 5
#define NUM_POSTS 20

// Comandos usados
char ERROR[]= "Erro.\n";
char message[]="MSG";
char nickname[]="NICK";
char user_info[]="INFO\n";
char add_tag[]="TAG";
char exit_chat[]="EXIT\n";
char user[]="USER";
char post[]="POST";
char read_post[]="READ";
char file[]="FILE";

char all_posts[NUM_POSTS][MAXLINE];
int count_post=0;

// Struct que representa os utilizadores 
struct List_c{
    int socket_num;                     // socket id
    int first_command;                  // usado para ver se o primeiro input que o cliente faz é o NICK, usado posteriormente
    char nick[NICKNAME];                // nickname associado ao cliente
    char ip[40];                        // ip associado ao cliente
    int port;                           // porta do server usada
    char tag[NUM_TAG][TAGSIZE];         // matriz de tags de um determinado utilizador
    int num_tag;                        // número de tags de um determinado utilizador
}list_c[MAX_CLIENT];


int pushClient(int connfd, char* c_ip, int c_port){    

    int i;
    for(i=0;i<MAX_CLIENT;i++){
        if(list_c[i].socket_num==INVALID_SOCK){
            list_c[i].socket_num = connfd;
            strcpy(list_c[i].ip,c_ip);
            list_c[i].port=c_port;
            strcpy(list_c[i].tag[0],"GLOBAL");
            list_c[i].num_tag=1;
            return i;
        }
    }

    if(i == MAX_CLIENT)
        return -1;
}

int popClient(int s)
{  
    for(int i=0; i<MAX_CLIENT;i++){
        if(s==list_c[i].socket_num){
            list_c[i].socket_num=INVALID_SOCK;
            memset(list_c[i].nick,0,sizeof(list_c[i].nick));
            memset(list_c[i].ip,0,sizeof(list_c[i].ip));
            break;
        }
    }
    close(s);
    return 0;
}

void exit_func(int i){

    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    printf("%s saiu do chat.\n",list_c[i].nick);
    for(int j=0; j<MAX_CLIENT;j++){
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"%s saiu do chat.\n",list_c[i].nick);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
    }
}

void set_nick(char* c_nick, int i){
    strcpy(list_c[i].nick,c_nick);
}

void nick_func(char* chatData, int i){

    char* token;
    char old_nick[NICKNAME];
    char buf1[MAXLINE];
    char* end = " ";
    int tamanho, aux = 0;

    memset(buf1,0,sizeof(buf1));

    token = strtok(chatData, end);

    if(strcmp(token, nickname)==0){

        token = strtok(NULL, end);
        token[strcspn(token, "\n")] = 0;

        if(strlen(token) == 0){
            if(list_c[i].socket_num!=INVALID_SOCK){
                printf("ERR NICK '%s'\n", token);
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }else{

            if(list_c[i].socket_num!=INVALID_SOCK){

                if(strlen(token) > NICKNAME){
                    // verificar se o nick definido ultrapassa o limite
                    printf("ERR NICK '%s'\n", token);
                    write(list_c[i].socket_num,buf1,strlen(buf1));

                }else if(strlen(token) < NICKNAME){
                    // verificar se existe algum cliente com o mesmo nome
                    for(int j = 0; j < MAX_CLIENT; j++){

                        if(strcmp(list_c[j].nick,token)==0){
                            printf("ERR NICK '%s'\n", token);
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            aux = aux + 1;
                            break;
                        }
                    }
                    if(aux == 0){

                        tamanho = strlen(list_c[i].nick);
                        if(tamanho > 0){
                            // substituir o nick de um cliente 
                            strcpy(old_nick,list_c[i].nick);
                            set_nick(token, i);
                            sprintf(buf1,"%s -> HELLO %s\n", old_nick, list_c[i].nick);               // <old_nick> ... <list_c[i].nick>  (client)
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            printf("OK NICK %s (OLD NICK: %s)\n", list_c[i].nick, old_nick);          // OK NICK ... (server)
                        
                        }else{
                            // um cliente novo no server
                            set_nick(token, i);
                            sprintf(buf1,"HELLO %s\n", list_c[i].nick);     // HELLO ... (client)
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            printf("OK NICK %s\n", list_c[i].nick);         // OK NICK ... (server)
                        }
                    }
                }
            }
        }
    }else{
        if(list_c[i].socket_num!=INVALID_SOCK){
            printf("ERR NICK '%s'\n", token);
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
    }
}

void addtag(char* chatData, int i){

    int decisao1 = 0;
    char* token=NULL;
    char buf1[MAXLINE];
    char aux2[20];
    char aux3[20];
    char pass[20];
    char* end = " ";

    token=strtok(chatData, end);
    memset(buf1,0,sizeof(buf1));

    if (list_c[i].num_tag < NUM_TAG){
        
        if (strcmp(token,add_tag) == 0){
            token = strtok(NULL, end);
            strcpy(aux2,token);
            token = strtok(NULL, end);
            token[strcspn(token, "\n")] = 0;
            strcpy(aux3,token);
            list_c[i].num_tag+=1;
            printf("%d\n", list_c[i].num_tag);
            int index=list_c[i].num_tag-1;
            strcpy(list_c[i].tag[index],aux3);
        }
        else{
            decisao1 = 1;
        }
    }
    else{
        write(list_c[i].socket_num,buf1,strlen(buf1));
        puts("ERR - Número máximo de tags atingido\n");
    }

    if (decisao1==1){
        write(list_c[i].socket_num,buf1,strlen(buf1));
        puts("ERR - O nick não está na lista de utilizadores ou foi introduzido incorretamente\n");
    }
}

void user_tag(int i){

    int cnt=0;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    for(int j=0; j<MAX_CLIENT;j++)
        if(list_c[j].socket_num!=INVALID_SOCK)
            cnt++;
    sprintf(buf1,"[Número de utilizadores no chat: %d]\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));
    
    for(int j=0; j<MAX_CLIENT;j++){
        if(list_c[j].socket_num!=INVALID_SOCK){
            for (int d = 0; d < list_c[j].num_tag; d++)
            {
                sprintf(buf1,"[Utilizador-> %s : TAG-> %s]\n",list_c[j].nick,list_c[j].tag[d]);
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }
    }   
}

int message_func(char* chatData, int i){         

    char* token;
    char buf1[MAXLINE];
    char* end = " ";

    memset(buf1,0,sizeof(buf1));
    token = strtok(chatData, end);                          // MSG

    if(strcmp(token, message)==0){

        token = strtok(NULL, end);                          // tag
        if (!strcmp(token,user))                            // ver se a tag é igual a USER
        {
            token = strtok(NULL, end);                      // username
            for (int j = 0; j < MAX_CLIENT; j++)
            {
                if (i!=j && !strcmp(list_c[j].nick,token)){
                    return 1;
                }
            }
        }
        else{
            
            for (int j = 0; j < MAX_CLIENT; j++)
            {
                for (int h = 0; h < list_c[j].num_tag; h++)
                {
                    if (i!=j && !strcmp(list_c[j].tag[h],token)){
                        return 1;
                    }
                }
            }
        }
    }else{ 
        if(list_c[i].socket_num!=INVALID_SOCK){
            //write(list_c[i].socket_num,buf1,strlen(buf1));
            return 0;
        }
    }
}

void remove_MSG_command(char* str, const char* toRemove)
{
    int len, removeLen, found = 0;

    len = strlen(str);
    removeLen = strlen(toRemove);

    for(int i=0; i<len; i++)
    {
        found = 1;
        for(int j=0; j<removeLen; j++)
        {
            if(str[i+j] != toRemove[j])
            {
                found = 0;
                break;
            }
        }

        if(found == 1)
        {
            for(int j=i; j<=len-removeLen; j++)
            {
                str[j] = str[j + removeLen + 1];
            }

            break;
        }
    }
}

char* remove_TAG(char* str){

    char *tag = malloc(sizeof(char)*TAGSIZE);
    char* token;
    char username[20];
    char mensage[20];
    char* end = " ";

    token=strtok(str, end);                         // primeiro nome da tag
    strcpy(tag,token);                              // copia o nome da tag para a variavel tag
 
    if (!strcmp(token,user))
    {
        token = strtok(NULL, end);                  // nome de utilizador
        strcpy(username,token);
        strcat(tag," ");
        strcat(tag,username);

        for (int i = 0; i <= strlen(str); i++)
            str[i]=str[strlen(tag)+i+1];

    }
    else{
        for (int i = 0; i <= strlen(str); i++)
            str[i]=str[strlen(tag)+i+1];
    }

    return tag;
}

void send_message(char* message, char* tag, int i){

    char* token;
    char* end=" ";
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    token=strtok(tag, end);
    if (!strcmp(token,user))
    {
        token = strtok(NULL, end);
        for (int j = 0; j < MAX_CLIENT; j++)
        {
            if (i!=j && !strcmp(list_c[j].nick,token))
            {
                sprintf(buf1,"%s : %s", list_c[i].nick,message);
                printf("TEXT %s %s %s",list_c[i].nick, list_c[j].nick, message);
                write(list_c[j].socket_num,buf1,strlen(buf1));
            }
        }
    }
    else{
        
        for (int j = 0; j < MAX_CLIENT; j++)
        {
            for (int d = 0; d < list_c[j].num_tag; d++)
            {
                if (i!=j && !strcmp(list_c[j].tag[d],tag))
                {
                    sprintf(buf1,"%s : %s", list_c[i].nick,message);
                    printf("TEXT %s %s %s",list_c[i].nick, list_c[j].tag[d], message);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }
}

void add_post(char* line, int i){

    char* token;
    char* end=" ";
    char *line_expt_command = malloc(sizeof(char)*CHATDATA);

    token=strtok(line, end);
    strcpy(line_expt_command,token);

    token=strtok(NULL, end);

    printf("%s\n", token);

    if (!strcmp(token,"GLOBAL"))
    {
        printf("OK POST %s\n", token);
        strcat(line_expt_command," ");
        strcat(line_expt_command,token);

        for (int d = 0; d <= strlen(line); d++)
            line[d]=line[strlen(line_expt_command)+d+1];

        printf("line: %s",line);
        strcpy(all_posts[count_post],line);
        printf("all_posts: %s",all_posts[count_post]);
        count_post+=1;

    }
    else{
        printf("ERR POST %s\n", token);
    }
}

void read_all_posts(char* line, int i){

    char* token;
    char* end=" ";
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    token=strtok(line, end);
    token=strtok(NULL, end);
    token[strcspn(token, "\n")] = 0;

    if (!strcmp(token,"GLOBAL"))
    {
        if (count_post!=0)
        {
            printf("OK ALLPOSTS %s %d\n", token, count_post);
            for (int d = 0; d < count_post; d++)
            {
                sprintf(buf1,"POST %d %s", d+1,all_posts[d]);
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }
        else{
            printf("ERR ALLPOSTS %s\n", token);
        }
    }
}

void send_file(char* line, int i){

    char* token;
    char* end=" ";
    char *filename = malloc(sizeof(char)*CHATDATA);
    char *tag_name = malloc(sizeof(char)*NICKNAME);
    char *bytes = malloc(sizeof(char)*CHATDATA);
    char *tag = malloc(sizeof(char)*TAGSIZE);

    token=strtok(line, end);        // FILE
    token=strtok(NULL, end);        // USER

    strcpy(tag,token);
    
    if (!strcmp(token,"USER"))
    {
        token=strtok(NULL, end);        // nome de utilizador
        strcpy(tag_name,token);
        for (int j = 0; j < MAX_CLIENT; j++)
        {
            if (!strcmp(tag_name,list_c[j].nick))
            {
                int sock=list_c[j].socket_num;
                token=strtok(NULL, end);        // filename
                strcpy(filename,token);
                token=strtok(NULL, end);        // bytes
                strcpy(bytes,token);
                int b = atoi(bytes);

                if (b<=CHATDATA)
                {
                    FILE *fp;
                    fp=fopen(filename,"r");
                    if (fp==NULL)
                    {
                        perror("ERR ao ler o ficheiro.\n");
                        exit(1);
                    }
                    
                    char buffer[CHATDATA]={0};
                    while (fgets(buffer,CHATDATA,fp)!=NULL)
                    {
                        if (send(sock,buffer,sizeof(buffer),0)==-1)
                        {
                            perror("ERR ao enviar o ficheiro.\n");
                            exit(1);
                        }
                        bzero(buffer,CHATDATA);  
                    }
                    printf("OK FILE USER %s %s\n", tag_name, filename);
                    printf("FILEFROM %s %s %d\n",list_c[i].nick,tag_name,b);
                    break;
                }
                else
                {
                    printf("ERR FILE USER %s %s\n", tag_name, filename);
                    break;
                }
            }
            else{
                printf("ERR FILE USER %s %s\n", tag_name, filename);
                break;
            }
        }
    }
    else{
        printf("ERR FILE USER %s %s\n", tag, filename);
    }
}

void main(int argc, char *argv[]) {

    int newSockfd, sockfd, maxfd=0, n;                  
    struct sockaddr_in servaddr;
    fd_set rset;

    int index;

    char* end = " ";
    char* token=NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char chatData[CHATDATA];
    char new_chatData[CHATDATA];
    char *message2;
    char *message3;


    if(argc<2){
        printf("usage: %s port_number\n",argv[0]);
        exit(-1);
    }

    // criação da socket
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0)) <= 0){
        perror("socket");
        exit(1);
    }

    memset(&servaddr,0,sizeof(servaddr));        //guarda as informações dos clientes
    servaddr.sin_addr.s_addr=INADDR_ANY;
    servaddr.sin_family=AF_INET;                 //endereço IPV4
    servaddr.sin_port=htons(atoi(argv[1]));      //atribuição da porta

    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
        printf("Error : Kill process.\n");
        exit(1);
    }

    if(listen(sockfd, MAX_CLIENT) < 0){
        printf("listen");
        exit(1);
    }

    for(int i=0;i<MAX_CLIENT;i++)
        list_c[i].socket_num=INVALID_SOCK;

    memset(buf1,0,sizeof(buf1));

    // Converte o endereço IP binário em um endereço IP decimal e armazena em buf1, guarda a porta onde o cliente entrou
    inet_ntop(AF_INET, &servaddr.sin_addr, buf1, sizeof(buf1));
    int save_port = (int) ntohs(servaddr.sin_port);

    while(1)
    {
        maxfd=sockfd;
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);

        for(int i=0; i<MAX_CLIENT; i++){
            if(list_c[i].socket_num!=INVALID_SOCK){
                FD_SET(list_c[i].socket_num,&rset);
                if(list_c[i].socket_num > maxfd) maxfd=list_c[i].socket_num;
            }
        }
        maxfd++;

        if(select(maxfd, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0){
            printf("Select error\n");
            exit(1);
        }

        int addrlen = sizeof(servaddr);

        if(FD_ISSET(sockfd, &rset)){
            // Aguardar até que a conexão com o cliente seja estabelecida
            // Quando aceite, um novo socket denominado newSockfd é criado. Envia e recebe os dados usando o NewSockFd
            if((newSockfd=accept(sockfd, (struct sockaddr *)&servaddr , (socklen_t *) &addrlen)) > 0) {

                memset(buf2, 0, sizeof(buf2));

                inet_ntop(AF_INET, &servaddr.sin_addr, buf2, sizeof(buf2));
             
                index = pushClient(newSockfd, buf2, save_port); //insere um novo cliente usando a função criada anteriormente PushCliente


                //mensagens de boas vindas dadas ao cliente
                message3 = "\n\nBEM-VINDO! \n\nDEFINE O TEU NICKNAME PARA COMEÇAR --> NICK <nickname>  \n\n";

                message2 = "\n                                   MENU                                         \n\n NICK <nickname> --> ATRIBUI/ALTERA O NICK DO UTILIZADOR \n\n MSG <tag> <message> --> ENVIA A MENSAGEM <message> PARA A TAG <tag> (GLOBAL OU USER <NICKNAME>) \n\n TAG <nickname> <tag> --> ADICIONA A TAG <tag> ÀS TAGS DO UTILIZADOR <nickname> \n\n INFO --> LISTA E MOSTRA A INFORMAÇÃO DOS UTILIZADORES NO CHAT (NICKNAME : TAG) \n\n POST <tag> <message> --> ENVIA UM POST COM A MENSAGEM <message> PARA A TAG <tag> (GLOBAL APENAS) \n\n READ <tag> --> PEDE AO SERVIDOR A LISTA DE POST JÁ ENVIADA PARA A TAG <tag> \n\n FILE <tag> <filename> <bytes> --> INICIA O ENVIO DO FICHEIRO <filename> , CUJO TAMANHO É DE <bytes> , PARA A TAG <tag> (USER APENAS) \n\n EXIT --> PARA O UTILIZADOR SAIR DO CHAT";


                //envia às messagens ao cliente, se por algum motivo não forem enviadas é mostrado um erro
                if( send(newSockfd, message2, strlen(message2), 0) != strlen(message2) ) {
                    
                    perror("send failed");
                }

                if( send(newSockfd, message3, strlen(message3), 0) != strlen(message3) ) {
                    
                    perror("send failed");
                }

                //mostra apenas na tela do servidor que foi inserido um novo utilizador
                puts("\nAguardando por uma ligação bem sucedida de um cliente...\n");
                
                if(index < 0){
                    write(newSockfd,ERROR,strlen(ERROR));
                    close(newSockfd);
                }
            }
        }

        for(int i=0; i<MAX_CLIENT;i++){
            if((list_c[i].socket_num != INVALID_SOCK) && FD_ISSET(list_c[i].socket_num,&rset)){

                memset(chatData, 0, sizeof(chatData));
                if((n=read(list_c[i].socket_num,chatData, sizeof (chatData)))>0){

                    // NICK = adiciona o nickname ao cliente
                    if(strstr(chatData, nickname) != NULL){
                        nick_func(chatData, i);
                        list_c[i].first_command = 1; 
                        continue;
                    }

                    // Vê se o comando NICK é o primeiro comando inserido (tem que ser obrigatoriamente)
                    if(list_c[i].first_command != 1){
                        memset(buf2, 0, sizeof(buf2));
                        sprintf(buf2,"O comando NICK deve ser o primeiro a ser inserido!\n");
                        write(list_c[i].socket_num,buf2,strlen(buf2));
                        continue;

                    }else{
                                              
                        // INFO = mostra a informação dos utilizadores conectados (NICKNAME/TAG)
                        if(!strcmp(chatData,user_info)){ 
                            user_tag(i);
                            continue;
                        }
                        
                        // MSG = troca de mensagens entre os utilizadores com uma determinada tag
                        if(strstr(chatData, message) != NULL){
                            char* auxiliar;
                            strcpy(new_chatData, chatData);
                            remove_MSG_command(chatData, message);
                            auxiliar=remove_TAG(chatData);
                            //printf("auxiliar: %s\n",auxiliar);
                            //printf("chatData: %s",chatData);
                
                            if (message_func(new_chatData, i)==1)
                            {
                                char* auxiliar2=malloc(sizeof(char)*TAGSIZE);
                                strcpy(auxiliar2,auxiliar);
                                printf("OK MSG %s\n", auxiliar2);
                                send_message(chatData,auxiliar,i);
                            }
                            else{
                                printf("ERR MSG %s\n", auxiliar);
                                //write(list_c[j].socket_num,buf1,strlen(buf1));
                            }
                            continue;
                        }
                        
                        
                        // TAG = adiciona uma tag ao utilizador
                        if(strstr(chatData, add_tag)!=NULL){
                            addtag(chatData,i);
                            continue;
                        }

                        // POST = envio de posts para a tag GLOBAL
                        if (strstr(chatData, post)!=NULL){
                            add_post(chatData,i);
                            continue;
                        }

                        // READ = leitura dos posts efetuados
                        if (strstr(chatData, read_post)!=NULL){
                            read_all_posts(chatData,i);
                            continue;
                        }

                        // FILE = inicia o envio do ficheiro <filename>, cujo tamanho é de <bytes>, para a tag <tag>
                        if (strstr(chatData, file)!=NULL){
                            send_file(chatData,i);
                            continue;
                        }
                        
                        // EXIT = utilizador saiu do chat
                        if(!strcmp(chatData,exit_chat)){ 
                            exit_func(i);
                            popClient(list_c[i].socket_num);
                            continue;
                        }
                    }
                }
            }
        }
    }
}