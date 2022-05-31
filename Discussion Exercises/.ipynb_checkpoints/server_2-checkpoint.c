// ICS53 - Assignment 5 Exercises

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<unistd.h>
// #include "Md5.c"  // Feel free to include any other .c files that you need in the 'Server Domain'.
#define PORT 9999

// Sending and receiving a simple message.
int server_example_1(client_socket, server_socket){
    ssize_t sent_size;
    char buffer[1024];  // Receiving the message into the buffer.
    int received_size;
    char* message = "Hello from your server";  // Message to send (22-characters/bytes).

    received_size = recv(client_socket, buffer, 1024, 0);
    printf("Server: received '%s' message with size = %i.\n", buffer, received_size);

    sent_size = send(client_socket, message, strlen(message), 0);
    printf("Server: message sent with size %d = %d\n", sent_size, strlen(message));

    // Make sure to close the sockets after you close the application.
    close(client_socket);
    close(server_socket);
    return 0;
}

// Sending and receiving multiple messages message.
int server_example_2(client_socket, server_socket){
    char buffer[1024];
    while (1){  // We go into an infinite loop because we don't know how many messages we are going to receive.
        int received_size = recv(client_socket, buffer, 1024, 0);
        if (received_size == 0){  // Socket is closed by the other end.
            close(client_socket);
            close(server_socket);
            break;
        }
        printf("Received %s from client with size = %i\n", buffer, received_size);

    }
    return 0;
}

// Sending and receiving an entire file.
int server_example_3(client_socket, server_socket){
    FILE *fptr;
    int chunk_size = 1000;
    char file_chunk[chunk_size];
    char source_path[] = "/Remote Directory/server_file.txt";

    fptr = fopen(source_path,"rb");  // Open a file in read-binary mode.
    fseek(fptr, 0L, SEEK_END);  // Sets the pointer at the end of the file.
    int file_size = ftell(fptr);  // Get file size.
    printf("Server: file size = %i bytes\n", file_size);
    fseek(fptr, 0L, SEEK_SET);  // Sets the pointer back to the beginning of the file.

    int total_bytes = 0;  // Keep track of how many bytes we read so far.
    int current_chunk_size;  // Keep track of how many bytes we were able to read from file (helpful for the last chunk).
    ssize_t sent_bytes;

    while (total_bytes < file_size){
        // Clean the memory of previous bytes.
        // Both 'bzero' and 'memset' works fine.
        bzero(file_chunk, chunk_size);
//        memset(file_chunk, '\0', chunk_size);

        // Read file bytes from file.
        current_chunk_size = fread(&file_chunk, sizeof(char), chunk_size, fptr);

        // Sending a chunk of file to the socket.
        sent_bytes = send(client_socket, &file_chunk, current_chunk_size, 0);

        // Keep track of how many bytes we read/sent so far.
//        total_bytes = total_bytes + current_chunk_size;
        total_bytes = total_bytes + sent_bytes;

        printf("Server: sent to client %i bytes. Total bytes sent so far = %i.\n", sent_bytes, total_bytes);

    }
    close(client_socket);
    close(server_socket);
    fclose(fptr);
    return 0;
}


// server_example_4:
// sending and receiving multiple files along with their file names.
// You have to do this yourself.
// Hint: you may want to attach a header to each file you send.


int start_server_2()
{
    int client_socket, server_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);


    // Creating socket file descriptor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int socket_status = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt));
    if (socket_status) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = INADDR_ANY;
    // The server IP address should be supplied as an argument when running the application.
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    int bind_status = bind(server_socket, (struct sockaddr*)&address, sizeof(address));
    if (bind_status < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int listen_status = listen(server_socket, 1);
    if (listen_status < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    client_socket = accept(server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }


    ///////////// Start sending and receiving process //////////////


    server_example_1(client_socket, server_socket);
//    server_example_2(client_socket, server_socket);
//    server_example_3(client_socket, server_socket);
    return 0;

}
