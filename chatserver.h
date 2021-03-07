struct chatclient_node
{
    int socket_desc;
    char* username;
    struct chatclient_node *next;
};
// Method Definitions.
void *connection_handler(void *);
void broadcast(int, char*);
void debug_client_array();
void remove_client(int);
