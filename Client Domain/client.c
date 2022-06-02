// The 'client.c' code goes here.

#include<stdio.h>
#include<unistd.h>
#include "md5.c"
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
                    bzero(buffer,1024);  // Receiving the message into the buffer.
                    sent_size = send(client_socket, line, strlen(line), 0);
                    //printf("Client: message sent %s. Original message size = %d bytes.\n", line, strlen(line));
                    received_size = recv(client_socket, buffer, 1024, 0);
                    bzero(message,MAXLINE);
                    strncpy(message, buffer, received_size);
                    message[received_size] = '\0';
                        //printf("Client received '%s' with size = %i\n", message, received_size);
                    if (strcmp(message,"none") == 0){
                        printf("File [%s] could not be found in remote directory.\n",args[1]);
                    }
                    else if(strcmp(message,"lock")== 0){
                        printf("File [%s] is currently locked by another user.\n",args[1]);
                    }
                    else{
                        //appending mode
                        //printf("file is free\n");
                        while(1){
                            printf("Appending> ");
                            line_size = getline(&line, &len, fptr_cmds);
                            if(line[strlen(line)-1] == '\n'){
                                line[strlen(line)-1] = '\0';
                            }
                            printf("%s\n",line);
                            char *args[MAXARGC];
                            char cache[MAXARGC][MAXLINE];
                            parseCmd(line, args, cache);
                            if(strcmp(args[0], "pause") == 0){
                                int sec = atoi(args[1]);
                                sleep(sec);
                            }
                            else if(strcmp(args[0], "close") == 0){
                                char n[] = "end";
                                sent_size = send(client_socket, n, strlen(n), 0);
                                    //printf("send %s, with size = %d\n",n,sent_size);
                                bzero(buffer,1024);
                                received_size = recv(client_socket,buffer,1024,0);
                                break;
                            }
                            else{
                                sent_size = send(client_socket, line, strlen(line), 0);
                                    //printf("send %s, with size = %d\n",line,strlen(line));
                                bzero(buffer,1024);
                                received_size = recv(client_socket,buffer,1024,0);
                            }
                        }
                    }
                }
                else if(strcmp(args[0], "upload") == 0){
                    char fname[128];
                    strcpy(fname, args[1]);
                    fname[strlen(fname) - 1] = '\0'; // remove \n
                    char cmd[128];
                    bzero(cmd, 128);
                    strcpy(cmd, "upload ");
                    strcat(cmd, fname);
                    FILE *fptr;
                    int chunk_size = 1000;
                    char file_chunk[chunk_size];
                    char source_path[128];
                    strcpy(source_path, "Local Directory/");
                    strcat(source_path, fname);
                    if((fptr = fopen(source_path,"rb")) != NULL){
                        ssize_t sent_size = send(client_socket, cmd, strlen(cmd), 0);
                        printf("Client uploads: %s\n", fname);
                        // puts(cmd);
                        char res[80];
                        recv(client_socket, &res, 80, 0);
                        printf("Client: receive cmd is received\n");
                        fseek(fptr, 0L, SEEK_END);  // Sets the pointer at the end of the file.
                        int file_size = ftell(fptr);  // Get file size.
                        // printf("%d\n", file_size);
                        fseek(fptr, 0L, SEEK_SET);  // Sets the pointer back to the beginning of the file.
                        int total_bytes = 0;  // Keep track of how many bytes we read so far.
                        int current_chunk_size; 
                        ssize_t sent_bytes;
                        while (1){
                            bzero(file_chunk, chunk_size);
                            current_chunk_size = fread(&file_chunk, sizeof(char), chunk_size, fptr);
                            sent_bytes = send(client_socket, &file_chunk, current_chunk_size, 0);
                            total_bytes = total_bytes + sent_bytes;

                            printf("Client: sent to client %li bytes. Total bytes sent so far = %i.\n", sent_bytes, total_bytes);

                            recv(client_socket, &res, 80, 0);
                            if(total_bytes >= file_size){
                                send(client_socket, "UPLOAD_OVER", strlen("UPLOAD_OVER"), 0);

                                printf("Client: send UPLOAD_OVER\n");

                                break;
                            }
                            else printf("Client: continues to uploading\n");
                        }
                        fclose(fptr);
                    }
                    else{
                        printf("Failed to open [%s] in Local Directory\n", fname);
                    }
                }
                else if(strcmp(args[0], "download") == 0){
                    int file_size;
                    char fname[128];
                    char res[80];
                    strcpy(fname, args[1]);
                    fname[strlen(fname) - 1] = '\0'; // remove \n
                    char destination_path[128];
                    strcpy(destination_path, "Local Directory/");
                    strcat(destination_path, fname);
                    char cmd[128];
                    bzero(cmd, 128);
                    strcpy(cmd, "download ");
                    strcat(cmd, fname);
                    send(client_socket, cmd, strlen(cmd), 0);
                    printf("Client: send cmd to server.\n");
                    bzero(res, 80);
                    recv(client_socket, &res, 80, 0);
                    puts(res);
                    if(strncmp(res, "Cmd is received.", strlen("Cmd is received.")) == 0){
                        printf("Client: receives \"Cmd is received.\" from server.\n");
                        send(client_socket, "Start downloading", strlen("Start downloading"), 0);
                        printf("Client: sends \"Start downloading.\"");
                        // Opening a new file in write-binary mode to write the received file bytes into the disk using fptr.
                        int chunk_size = 1000;
                        FILE *fptr;
                        char file_chunk[chunk_size];
                        // Opening a new file in write-binary mode to write the received file bytes into the disk using fptr.
                        fptr = fopen(destination_path,"wb");
                        int bytes_received = 0;
                        // int i = 0;
                        // Keep receiving bytes until we receive the whole file.
                        while (1){
                            bzero(file_chunk, chunk_size);
                            // Receiving bytes from the socket.
                            file_size = recv(client_socket, file_chunk, chunk_size, 0);
                            
                            bytes_received += file_size;
                            printf("Client: received %i bytes from server.\n", file_size);
                            printf("Client: received \"%s\" from server.\n", file_chunk);
                            // The server has closed the connection.
                            // Note: the server will only close the connection when the application terminates.
                            
                            if(file_size == 0 ){
                                printf("Client: file size = 0, break.\n");
                                break;
                            }
                            if(strncmp(file_chunk, "DOWNLOAD_OVER", 11) == 0){
                                printf("Client: Download is over, break.\n");
                                break;
                            }
                            if (strncmp(file_chunk, "DOWNLOAD_OVER", 11) != 0){
                                fwrite(&file_chunk, sizeof(char), file_size, fptr);
                                printf("Client: write \"%s\" in file.\n", file_chunk);
                                send(client_socket, "File chunk received", strlen("File chunk received"), 0);
                                printf("Client: send \"File chunk received\"\n");
                            }
                            // exit(0);
                        }

                        printf("File [%s] is downloaded. [%d Bytes]\n", fname, bytes_received);
                        fclose(fptr);
                    }
                    else{
                        printf("Client: failed to download file from server.\n");
                    }
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
        puts("Connection closed.");
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