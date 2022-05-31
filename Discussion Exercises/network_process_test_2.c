// ICS53 - Assignment 5 Exercises

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "client_2.c"
#include "server_2.c"


int main()
{
    printf("Start...\n");
    pid_t server_pid = fork ();
    if (server_pid == 0){
        start_server_2();
        exit(0);
    }
    pid_t client_pid = fork ();
    if (client_pid == 0){
        start_client_2();
        exit(0);
    }
    waitpid (server_pid, NULL, 0);
    waitpid (client_pid, NULL, 0);
    printf("Exiting...\n");

    return 0;

}