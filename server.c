#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

int main (int argc, char *argv[]) {
    if (argc != 2){
        printf("server <server port>\n");
        return -1;
    }

    int port = atoi(argv[1]);

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[1024], client_message[1024];
    socklen_t client_struct_length = sizeof(client_addr);

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));


    if(socket_desc = socket(AF_INET, SOCK_DGRAM, 0)<0){
        printf("Error: socket()\n");
        return -1;
    }

    // Set port and IP
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding

    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Error: bind()\n");
        return -1;
    }

    // Receive the server's response:
    if(recvfrom(socket_desc, client_message, sizeof(client_message), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    // Respond to Client
    if(strcmp(client_message, "ftp")){
        strcpy(server_message, "yes");
    }
    else{
        strcpy(server_message, "no");
    }


    // Send the message to client:
    if(sendto(socket_desc, server_message, strlen(server_message), 0,
         (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    close(socket_desc);
    return 0;
}