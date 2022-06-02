// The 'server.c' code goes here.

#include<stdio.h>
#include<unistd.h>
#include "md5.c"  // Feel free to include any other .c files that you need in the 'Server Domain'.
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
        bzero(buffer, 1024);
        bzero(str, 1024);
        int received_size = recv(conn_fd, buffer, 1024, 0);
        send(conn_fd, "Cmd is received.\n", strlen("Cmd is received.\n"), 0);
        // Socket is closed by the other end.
        if (received_size == 0){
            close(conn_fd);
            close(server_fd);
            break;
        }
        strncpy(str, buffer, received_size);
        char *args[MAXARGC];
        char cache[MAXARGC][MAXLINE];
        parseCmd(str, args, cache);
        puts(str);
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
            // Msg format: #1, "upload [file name]"; #2, [FILE]
            int file_size;
            char fname[128];
            strcpy(fname, args[1]);
            char destination_path[128];
            strcpy(destination_path, "Remote Directory/");
            strcat(destination_path, fname);
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
                file_size = recv(conn_fd, file_chunk, chunk_size, 0);
                
                bytes_received += file_size;
                printf("Server: received %i bytes from client.\n", received_size);
                printf("Server: received \"%s\" from client.\n", file_chunk);
                // The server has closed the connection.
                // Note: the server will only close the connection when the application terminates.
                
                if(file_size == 0 ){
                    printf("Server: file size = 0, break.\n");
                    break;
                }
                if(strncmp(file_chunk, "UPLOAD_OVER", 11) == 0){
                    printf("Server: Upload is over, break.\n");
                    break;
                }
                if (strncmp(file_chunk, "UPLOAD_OVER", 11) != 0){
                    fwrite(&file_chunk, sizeof(char), file_size, fptr);
                    printf("Server: write \"%s\" in file.\n", file_chunk);
                    send(conn_fd, "File chunk received", strlen("File chunk received"), 0);
                    printf("Server: send \"File chunk received\"\n");
                }
                // exit(0);
            }

            printf("File [%s] is uploaded. [%d Bytes]\n", fname, bytes_received);
            fclose(fptr);
            
        }
        else if(strcmp(args[0], "download") == 0){

            char fname[128];
            strcpy(fname, args[1]);
            // fname[strlen(fname) - 1] = '\0'; // remove \n
            FILE *fptr;
            int chunk_size = 1000;
            char file_chunk[chunk_size];
            char source_path[128];
            strcpy(source_path, "Remote Directory/");
            strcat(source_path, fname);
            if((fptr = fopen(source_path,"rb")) != NULL){
                // send(conn_fd, "Ready to download", strlen("Ready to download"), 0);
                // printf("Server: send \"Ready to download\"\n");
                char res[80];
                bzero(res, 80);
                recv(conn_fd, &res, 80, 0);
                printf("Server: receive \"Start downloading\"\n");
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
                    sent_bytes = send(conn_fd, &file_chunk, current_chunk_size, 0);
                    total_bytes = total_bytes + sent_bytes;

                    printf("Server: sent to client %li bytes. Total bytes sent so far = %i.\n", sent_bytes, total_bytes);

                    recv(conn_fd, &res, 80, 0);
                    if(total_bytes >= file_size){
                        send(conn_fd, "DOWNLOAD_OVER", strlen("DOWNLOAD_OVER"), 0);

                        printf("Server: send DOWNLOAD_OVER\n");

                        break;
                    }
                    else printf("Server: continues to downloading\n");
                }
                fclose(fptr);
            }
            else{
                printf("Failed to open [%s] in Remote Directory\n", fname);
            }


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
