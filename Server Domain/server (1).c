// The 'server.c' code goes here.

#include<stdio.h>
#include<unistd.h>
#include "Md5.c"  // Feel free to include any other .c files that you need in the 'Server Domain'.
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h> 
#define MAXLINE 80
#define MAXARGC 80
#define PORT 9999

struct file{
    char fname[MAXLINE];
    char fpath[MAXLINE];
    char status[MAXLINE];// free or lock
    uint8_t md5;
};
struct file files[MAXLINE];

// Parse command into ARRAY args
void parseCmd(char *cmdline, char **args, char cache[][MAXLINE]){////
    char tmp[MAXLINE];////
    strcpy(tmp, cmdline);
    const char s[2] = " ";
    int i = 0;
    char *token = strtok(tmp, s);

    strcpy(cache[i], token);
    args[i] = cache[i];

    i++;
    while(token != NULL){
        token = strtok(NULL, s);
        if(token != NULL){
            strcpy(cache[i], token);
            args[i] = cache[i];
            i++;
        }
    }
    while(i < MAXARGC){////
        args[i++] = NULL;
    }
}

int main(int argc, char *argv[])
{
    //test code-----------------------------------------------------------
    strcpy(files[0].fname, "hello.txt");
    strcpy(files[0].fpath, "Remote Directory/hello.txt");
    strcpy(files[0].status, "free");

    strcpy(files[1].fname, "world.txt");
    strcpy(files[1].fpath, "Remote Directory/world.txt");
    strcpy(files[1].status, "lock");
    //test code-----------------------------------------------------------

    int server_fd, conn_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    struct sockaddr_storage clientaddr; /* Enough room for any addr */
    int opt = 1;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(argv[1]);
    address.sin_port = htons( PORT );
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == 0) {
        perror("socket failed"); 
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt"); exit(EXIT_FAILURE);
    }
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
          perror("bind failed");
          exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((conn_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
        perror("accept");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    char str[1024];
    // # following commands are supported
    // 1. getFnames
    // 2. upload [fname]
    // 3. download [fname]
    // 4. delete [fname]
    // 5. syncheck
    while (1){  // We go into an infinite loop because we don't know how many messages we are going to receive.
        bzero(buffer,1024);
        int received_size = recv(conn_fd, buffer, 1024, 0);
        if(received_size == 0){
            close(conn_fd);
            close(server_fd);
            break;
        }
        printf("buffer  %s\n",buffer);
        bzero(str,1024);
        strncpy(str, buffer, received_size);

        //printf("received %s, with size = %d\n",str, received_size);

        char *args[MAXARGC];
        char cache[MAXARGC][MAXLINE];
        parseCmd(str, args, cache);
        if(strcmp(args[0], "append") == 0){
            ssize_t sent_size;
            char result[MAXLINE];
            int n = findfilename(result,args[1],files);
            sent_size = send(conn_fd, result,strlen(result), 0);
            
            //printf("server: message sent %s with size = %d\n",result,sent_size);
            
            if (strcmp(result,"free") == 0){
                changestatuslock(args[1],files);
                char p [MAXLINE];
                findp(p,args[1],files);
                int n = 5;
                while(n > 0){
                    bzero(buffer,1024);
                    received_size = recv(conn_fd, buffer,1024,0);
                    //printf("server receive: %s\n",buffer);
                    if(strcmp(buffer,"end")==0){
                        changestatusfree(args[1], files);
                        sent_size = send(conn_fd, "end",strlen("end"), 0);
                        n = 0;
                    }
                    else{
                        FILE *fp;
                        fp = fopen( p , "a" );
                        strcat(buffer,"\n");
                        fwrite(buffer , sizeof(char), received_size+1 , fp );
                        fclose(fp);
                        sent_size = send(conn_fd, "add",strlen("add"), 0);
                    }
                    n--;
                } 
            }
        }
        else if(strcmp(args[0], "upload") == 0){
            
        }
        else if(strcmp(args[0], "download") == 0){

        }
        else if(strcmp(args[0], "delete") == 0){
            ssize_t sent_size;
            bzero(buffer,1024);
            deletefile(buffer,args[1],files);
            sent_size = send(conn_fd, buffer,strlen(buffer), 0);
        }
        else if(strcmp(args[0], "syncheck") == 0){
            
        }
    }
    return 0;
}

//return status if found, or return 0;
int findfilename(char * result, char *filename, struct file files[MAXLINE]){
    int i;
    for (i = 0 ; i < MAXLINE; i++){
        if(strcmp(filename,files[i].fname) == 0){
            strcpy(result, files[i].status);
            return 0;
        }
    }
    strcpy(result,"none");
    return 1;
}

void changestatuslock(char * filename, struct file files[MAXLINE]){
    int i;
    for (i = 0 ; i < MAXLINE; i++){
        if(strcmp(filename,files[i].fname) == 0){
            strcpy(files[i].status, "lock");
            break;
        }
    }
}

void changestatusfree(char * filename, struct file files[MAXLINE]){
    int i;
    for (i = 0 ; i < MAXLINE; i++){
        if(strcmp(filename,files[i].fname) == 0){
            strcpy(files[i].status, "free");
            break;
        }
    }
}

void findp(char * p, char *filename, struct file files[MAXLINE]){
    int i;
    for (i = 0 ; i < MAXLINE; i++){
        if(strcmp(filename,files[i].fname) == 0){
            strcpy(p, files[i].fpath);
            break;
        }
    }
}

void deletefile(char * result, char * filename, struct file files[MAXLINE]){
    int i;
    for (i = 0 ; i < MAXLINE; i++){
        char p[MAXLINE];
        if(strcmp(filename,files[i].fname) == 0){
            strcpy(p, files[i].fpath);
            strcpy(files[i].fname,"none");
            remove(p);
            strcpy(result, "success");
            break;
        }
        else{
            strcpy(result, "fail");
        }
    }
}