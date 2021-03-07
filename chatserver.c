#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h>
#include "chatserver.h"

//Golbal variables
struct chatclient_node *head = NULL;
struct chatclient_node *tail = NULL;
int num_clients = 0;
 
int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c, *new_sock;
    struct sockaddr_in server , client;
    char *message;
    int port = atoi(argv[3]);
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);
     
    //Bind
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\n%d\n", socket_desc);
        printf("%d\n", port);
        puts("bind failed");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 100);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        //Creating new client struct for client array.
        char *temp = (char*) malloc(2000);
        size_t msg_length = 0;
        if(msg_length = read(new_socket , temp , 2000) < 0) {
            puts("Error: Error getting username from client");
            return 1;
        }

        //Creating linked list node

        struct chatclient_node *new_client;
        new_client = (struct chatclient_node *) malloc(sizeof(struct chatclient_node));
        new_client->socket_desc = new_socket;
        new_client->username = temp;
        new_client->next = NULL;
        if(head == NULL || tail == NULL) {
            head = new_client;
            tail = new_client;
        } else {
            tail->next = new_client;
            tail = tail->next;
        }
        num_clients++;
        printf("\n%s has joined!\n", new_client->username);
        // debug_client_array();

        //Multithreading to assign a handler to each client.
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
    }
     
    if (new_socket < 0)
    {
        perror("accept failed");
        return 1;
    }
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[1024];
    struct chatclient_node *new_client = tail;

    // Broadcasting new client to all other clients
    char broadcast_message[200];
    const char* format_string = "%s has joined!";
    sprintf(broadcast_message, format_string, new_client->username);
    broadcast(new_client->socket_desc, broadcast_message);

    while(1)
    {
        read_size = recv(sock , client_message , 2000 , 0);
        if(read_size == -1)
        {
            puts("Error: Read from client error.");
        }
        if (!read_size|| !strcmp(client_message, ":exit")) {
            char exit_message[200];
            sprintf(exit_message, "%s has left the chatroom.", new_client->username);
            puts(exit_message);
            broadcast(sock, exit_message);
            remove_client(sock);
            debug_client_array();
            break;
        }
        client_message[read_size] = '\0';
        puts(client_message);
        broadcast(sock, client_message);
    }
    free(socket_desc);
     
    return 0;
}

void broadcast(int sender_socket_desc, char* message) {
    struct chatclient_node *temp = head;
    for(int i = 0; i < num_clients; i++) {
        if(temp->socket_desc != sender_socket_desc){
            write(temp->socket_desc , message , strlen(message));
        }
        temp = temp->next;
    }
}

void debug_client_array() {
    struct chatclient_node *temp = head;
    for(int i = 0; i < num_clients; i++) {
        printf("-> U: %s, Socket: %d", temp->username, temp->socket_desc);
        temp = temp->next;
    }
}

void remove_client(int socket) {
    struct chatclient_node *temp = head;
    if (num_clients == 1) {
        head = tail = NULL;
        num_clients--;
        return;
    }
    while(temp->next && temp->next->socket_desc != socket) {
        temp = temp->next;
    }
    if (temp->next) {
        struct chatclient_node *temp2 = temp->next;
        temp->next = temp->next->next;
        free(temp->next);
        num_clients--;
    }
}