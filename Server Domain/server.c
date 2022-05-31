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
            int file_size;
            recv(conn_fd, buffer, 1024, 0);
            char fname[128];
            strncpy(fname, buffer, received_size + 1);
            printf("File [%s] is uploaded\n", fname);
            // recv(conn_fd, buffer, 1024, 0);
            FILE *fptr;
            int chunk_size = 1000;
            char file_chunk[chunk_size];
            
            char destination_path[128];
            strcpy(destination_path, "Remote Directory/");
            strcat(destination_path, fname);
            // Opening a new file in write-binary mode to write the received file bytes into the disk using fptr.
            fptr = fopen(destination_path,"wb");

            // Keep receiving bytes until we receive the whole file.
            while (1){
                bzero(file_chunk, chunk_size);
                // Receiving bytes from the socket.
                file_size = recv(conn_fd, file_chunk, chunk_size, 0);
                printf("Client: received %i bytes from server.\n", received_size);

                // The server has closed the connection.
                // Note: the server will only close the connection when the application terminates.
                if (file_size == 0){
                    // close(conn_fd);
                    fclose(fptr);
                    break;
                }
                // Writing the received bytes into disk.
                fwrite(&file_chunk, sizeof(char), file_size, fptr);
            }
            
        }
        else if(strcmp(args[0], "download") == 0){
            // Msg format: #1, "download" (received); #2, file name
            // Existence of file was checked in client server
            recv(conn_fd, buffer, 1024, 0);
            char fname[128];
            strncpy(fname, buffer, received_size + 1);
            FILE *fptr;
            int chunk_size = 1000;
            char file_chunk[chunk_size];
            
            char source_path[128];
            strcpy(source_path, "Remote Directory/");
            strcat(source_path, fname);
            fptr = fopen(source_path,"rb");  // Open a file in read-binary mode.
            fseek(fptr, 0L, SEEK_END);  // Sets the pointer at the end of the file.
            int file_size = ftell(fptr);  // Get file size.
            // printf("Server: file size = %i bytes\n", file_size);
            fseek(fptr, 0L, SEEK_SET);  // Sets the pointer back to the beginning of the file.
            int total_bytes = 0;  // Keep track of how many bytes we read so far.
            int current_chunk_size;  // Keep track of how many bytes we were able to read from file (helpful for the last chunk).
            ssize_t sent_bytes;
            while (total_bytes < file_size){
                // Clean the memory of previous bytes.
                bzero(file_chunk, chunk_size);
                // Read file bytes from file.
                current_chunk_size = fread(&file_chunk, sizeof(char), chunk_size, fptr);
                // Sending a chunk of file to the socket.
                sent_bytes = send(conn_fd, &file_chunk, current_chunk_size, 0);
                // Keep track of how many bytes we read/sent so far.
                total_bytes = total_bytes + sent_bytes;
                printf("Server: sent to client %i bytes. Total bytes sent so far = %i.\n", sent_bytes, total_bytes);
            }
            
        }
        else if(strcmp(args[0], "delete") == 0){
            // Msg format: #1, "delete" (received); #2, file name
            // Existence of file was checked in client server
            recv(conn_fd, buffer, 1024, 0);
            char fname[128];
            strncpy(fname, buffer, received_size + 1);
            char delete_path[128];
            strcpy(delete_path, "Remote Directory/");
            strcat(delete_path, fname);
            remove(delete_path);
            // char msg[128];
            // Should I send msg to indicate that the file has been removed?
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
