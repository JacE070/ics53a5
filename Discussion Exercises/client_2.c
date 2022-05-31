// ICS53 - Assignment 5 Exercises

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
//#include <sys/socket.h>
#include <unistd.h>
#define PORT 9999

// Sending and receiving a simple message.
int client_example_1(client_socket){
    ssize_t sent_size;
    char buffer[1024];  // Receiving the message into the buffer.
    int received_size;
    char message[] = "Hello from client";  // Message to send (17-characters/bytes).

    // Since client_socket is a File Descriptor, both "write" and "send" should work.
    // However, "send" is dedicated for sockets which gives you a fourth arguments to set some socket options.
//    sent_size = write(client_socket, message, strlen(message));
    sent_size = send(client_socket, message, strlen(message), 0);
    printf("Client: message sent with size %d bytes. Original message size = %d bytes.\n", sent_size, strlen(message));

    // Since client_socket is a File Descriptor, both "read" and "recv" should work.
    // However, "recv" is dedicated for sockets which gives you a fourth arguments to set some socket options.
//    received_size = read(client_socket, buffer, 1024);
    received_size = recv(client_socket, buffer, 1024, 0);
    printf("Client received '%s' with size = %i\n", buffer, received_size);

    // Make sure to close the socket after you close the application.
    close(client_socket);
    return 0;
}

// Sending and receiving multiple messages message.
int client_example_2(client_socket){

    char message1[] = "aaa";
    send(client_socket, message1, strlen(message1), 0);
    sleep(1);

    char message2[] = "bbbb";
    send(client_socket, message2, strlen(message2), 0);
    sleep(1);

    char message3[] = "ccccc";
    send(client_socket, message3, strlen(message3), 0);

    // Make sure to close the socket after you close the application.
    close(client_socket);
    return 0;
}

// Sending and receiving an entire file.
int client_example_3(client_socket){
    int received_size;
    char destination_path[] = "/Local Directory/received_file.txt";  // Note how we don't have the original file name.
    int chunk_size = 1000;
    char file_chunk[chunk_size];
//    int chunk_counter = 0;

    FILE *fptr;

    // Opening a new file in write-binary mode to write the received file bytes into the disk using fptr.
    fptr = fopen(destination_path,"wb");

    // Keep receiving bytes until we receive the whole file.
    while (1){
        bzero(file_chunk, chunk_size);
//        memset(&file_chunk, 0, chunk_size);

        // Receiving bytes from the socket.
        received_size = recv(client_socket, file_chunk, chunk_size, 0);
        printf("Client: received %i bytes from server.\n", received_size);

        // The server has closed the connection.
        // Note: the server will only close the connection when the application terminates.
        if (received_size == 0){
            close(client_socket);
            fclose(fptr);
            break;
        }
        // Writing the received bytes into disk.
        fwrite(&file_chunk, sizeof(char), received_size, fptr);
//        printf("Client: file_chunk data is:\n%s\n\n", file_chunk);
    }
}


// client_example_4:
// sending and receiving multiple files along with their file names.
// You have to do this yourself.
// Hint: you may want to attach a header to each file you send.


int start_client_2()
{
    int client_socket;
    struct sockaddr_in serv_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // The server IP address should be supplied as an argument when running the application.
    int addr_status = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    if (addr_status <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    int connect_status = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (connect_status < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    ///////////// Start sending and receiving process //////////////


    client_example_1(client_socket);
//    client_example_2(client_socket);
//    client_example_3(client_socket);
    return 0;


}
