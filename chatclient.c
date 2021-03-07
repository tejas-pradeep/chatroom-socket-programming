#include <string.h>
#include "chatclient.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
int loop_flag = 1;
void *recieving(void * sock) {
    int server_socket = *((int *) sock);
    while (loop_flag) {
        char message[200];
        int read_size = read(server_socket, message , 2000);
        if (read_size == 0) {
            loop_flag = 0;
            break;
        }
        message[read_size] = '\0';
        puts(message);
    }
}

int main(int argc , char *argv[])
{
    //Checking for errors with inputs.
    if (argc != 10) {
        printf("\nError: Command line input is missing required arguments. \nArguments required are: Hostname, Username, Password\n");
        return 1;
    }
    char *host_name = argv[3], *username = argv[7], *password = argv[9];
    int port = atoi(argv[5]);
    // Check for alpha numeric username
    char *temp = username;
    while(*temp != '\0') {
        if (isalnum(*temp) == 0) {
            printf("\nError: Username has non alphanumeric charecters.\n");
            return 1;
        }
        temp++;
    }
    if (strcmp(PASSWORD, password) != 0) {
        printf("\nError: Password is wrong\n");
        return 1;
    }

    if (strcmp(HOST, host_name) != 0) {
        printf("\nError: Invalid host name. The Hostname should be 127.0.0.1\n");
        return 1;
    }
    int socket_desc;
    struct sockaddr_in client;
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
         
    client.sin_addr.s_addr = inet_addr(HOST);
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
 
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&client , sizeof(client)) < 0)
    {
        puts("Error: Connection Error. Invalid port.Port should be 5001");
        return 1;
    }
    // Sending username to server.
    if(send(socket_desc , username , strlen(username) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("~~~ You have entered CS3251 Chat Room ~~~\n");

    pthread_t thread;
    pthread_create(&thread, NULL, recieving, (void *) &socket_desc);

    // Send messages to server
    char message[1024];
    int read_size;
    while(loop_flag) {
        fgets(message, 1024, stdin);
        char updated_message[1024];
        sprintf(updated_message, "%s: %s", username, message);
        send(socket_desc, updated_message, strlen(updated_message), 0);
    }
    return 0;
}
