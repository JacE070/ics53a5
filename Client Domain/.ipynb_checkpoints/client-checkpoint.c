// The 'client.c' code goes here.

#include<stdio.h>
#include<unistd.h>
#include "Md5.c"  // Feel free to include any other .c files that you need in the 'Server Domain'.
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAXLINE 80
#define MAXARGC 80
#define PORT 9999

void parseCmd(char *cmdline, char **args, char cache[][MAXLINE]);
void parseFname(char *cmdline, char **args, char cache[][MAXLINE]);
int strInArray(char *str, char* array[]);

int main(int argc, char * argv[])
{
    FILE *fptr_cmds;
    char fpath_cmds[80];
    strcpy(fpath_cmds, "");
    strcat(fpath_cmds, argv[1]);
    char* ip = argv[2];
    if((fptr_cmds = fopen(fpath_cmds,"r")) != NULL){
        char *line;
        size_t len;
        ssize_t line_size;
        
        // Start Client Server
        int client_socket;
        struct sockaddr_in serv_addr;

        client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (client_socket < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        int addr_status = inet_pton(AF_INET, ip, &serv_addr.sin_addr);
        if (addr_status <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }
        int connect_status = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (connect_status < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        
        
        while((line_size = getline(&line, &len, fptr_cmds)) > 0){
            char *args[MAXARGC];
            char cache[MAXARGC][MAXLINE];
            parseCmd(line, args, cache);
            // int arg_len = sizeof(args) / sizeof(args[0]);
            // printf("%d\n", strcmp(args[0], "append"));
            if(1){
                // puts(args[0]);
                if(strcmp(args[0], "pause") == 0){
                    int sec = atoi(args[1]);
                    sleep(sec);
                }
                else if(strcmp(args[0], "append") == 0){
                    // puts("IN");
                    char *fname = args[1];
                    // get file names in remote directory
                    ssize_t sent_size;
                    char fnames[1024];  // Receiving the message into the buffer.
                    int received_size;
                    char message[] = "getFnames";
                    sent_size = send(client_socket, message, strlen(message), 0);
                    // Fnames in format "<fname, [fname]>"
                    received_size = recv(client_socket, fnames, 1024, 0);
                    // puts(fnames);
                    char *args1[MAXARGC];
                    char cache1[MAXARGC][MAXLINE];
                    // Strip < and >
                    char *fnamesptr = &fnames[0];
                    while(*fnamesptr != '<'){
                        fnamesptr ++;
                    }
                    fnamesptr ++;
                    fnamesptr[strlen(fnameptr) - 1] = '\0';
                    // Put Fnames in args1
                    parseFname(fnamesptr, args1, cache1);
                    
                    if(!strInArray(fname, args1)){
                        printf("File [%s] could not be found in remote directory.", fname);
                        continue;
                    }
                    // Enter Append Mode
                    // Next line
                    getline(&line, &len, fptr_cmds);
                    parseCmd(line, args, cache);
                    if(strcmp(args[0], "pause") == 0){
                        sleep(atoi(args[1]));
                    }
                    else if(strcmp(args[0], "close") == 0){
                        break;
                    }
                    else{ // Append content, use line
                        // 1. Download file from server first
                        // 2. Append content to file
                        // 3. Upload file to server
                        
                    }
                    
                }
                else if(strcmp(args[0], "upload") == 0){
                    
                }
                else if(strcmp(args[0], "download") == 0){
                    
                }
                else if(strcmp(args[0], "delete") == 0){
                    
                }
                else if(strcmp(args[0], "syncheck") == 0){
                    
                }
                else if(strcmp(args[0], "quit") == 0){
                    close(client_socket);
                    break;
                }
                else if(strcmp(args[0], "test") == 0){
                    ssize_t sent_size;
                    char buffer[1024];  // Receiving the message into the buffer.
                    int received_size;
                    char message[] = "getFnames";
                    sent_size = send(client_socket, message, strlen(message), 0);
                    // Fnames in format "<fname, [fname]>"
                    received_size = recv(client_socket, buffer, 1024, 0);
                }
            }
            else{
                puts("Wrong number of arguments.\n");
                continue;
            }
        }
    }
    else{
        puts("File not found.");
    }
	return 0;
}


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

// Parse command into ARRAY args
void parseFname(char *cmdline, char **args, char cache[][MAXLINE]){////
    char tmp[MAXLINE];////
    strcpy(tmp, cmdline);
    const char s[2] = ",";
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

int strInArray(char *str, char* array[]){
    int i;
    for(i = 0; i < sizeof(*array)/sizeof(*array[0]); i++){
        if(strcmp(str, array[i]) == 0){
            return 1;
        }
    }
    return 0;
}