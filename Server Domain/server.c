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
    // # following commands are supported
    // 1. getFnames
    // 2. upload [fname]
    // 3. download [fname]
    // 4. delete [fname]
    // 5. syncheck
    while (1){  // We go into an infinite loop because we don't know how many messages we are going to receive.
        int received_size = recv(conn_fd, buffer, 1024, 0);
        char str[1024];
        strncpy(str, buffer, received_size + 1);
        char *args[MAXARGC];
        char cache[MAXARGC][MAXLINE];
        parseCmd(str, args, cache);
        if(strcmp(args[0], "getFnames") == 0){
            // Msg format: #1, "getFnames"
            DIR * d = opendir("Remote Directory/");
            struct dirent *dir;
            if(d){
                char fnames[1024];
                int i = 0;
                strcat(fnames, "<");
                while ((dir = readdir(d)) != NULL) {
                    if(i < 2){i++; continue;} // ignore "." and ".."
                    strcat(fnames, dir->d_name);
                    strcat(fnames, ",");
                    i++;
                }
                fnames[strlen(fnames) - 1] = '\0';
                strcat(fnames, ">");
                // Send fnames in format "<fname, [fname]>"
                ssize_t sent_size = send(conn_fd, fnames, strlen(fnames), 0);
                closedir(d);
            }
            else{
                puts("No d");
            }
        }
        else if(strcmp(args[0], "upload") == 0){
            // Msg format: #1, "upload" (received); #2, file name; #3, [FILE]
            int received_size = recv(conn_fd, buffer, 1024, 0);
            char fname[1024];
            strncpy(fname, buffer, received_size + 1);
            
            
        }
        else if(strcmp(args[0], "download") == 0){
            // Msg format: #1, "download" (received); #2, file name
            // Existence of file was checked in client server
            int received_size = recv(conn_fd, buffer, 1024, 0);
            char fname[1024];
            strncpy(fname, buffer, received_size + 1);
            
            
        }
        else if(strcmp(args[0], "delete") == 0){
            // Msg format: #1, "delete" (received); #2, [File]
            // Existence of file was checked in client server
            int received_size = recv(conn_fd, buffer, 1024, 0);
            char fname[1024];
            strncpy(fname, buffer, received_size + 1);
            
            
        }
        else if(strcmp(args[0], "syncheck") == 0){
            
        }
        
        // Socket is closed by the other end.
        if (received_size == 0){
            close(conn_fd);
            close(server_fd);
            break;
        }
    }
    return 0;
}
