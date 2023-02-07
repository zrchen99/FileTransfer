#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include "packet.h"

int main (int argc, char *argv[]) {
    if (argc != 2){
        printf("server <server port>\n");
        return -1;
    }

    int port = atoi(argv[1]);
    printf(argv[1]);

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[1024], client_message[1024];
    socklen_t client_struct_length = sizeof(client_addr);

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));


    if((socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0){
        printf("Error: socket()\n");
        return -1;
    }

    // Set port and IP
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding

    if((bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)))<0){
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
    if(strcmp(client_message, "ftp") == 0){
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

    //Lab 2: create packet and create txt file
    FILE *fptr;
    while(true){
        if(recvfrom(socket_desc, client_message, sizeof(client_message), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }
        struct packet *curr_packet = message_to_packet(client_message);
        if(curr_packet->frag_no == 1){
            fptr = fopen(curr_packet->filename,"w");
        }
        //write to file
        fwrite(curr_packet->filedata, sizeof(char), curr_packet->size, fptr);

        // ACK
        create_ack_message(&curr_packet, server_message);

        // ACK sending
        if(sendto(socket_desc, server_message, strlen(server_message), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        // end condition
        if (curr_packet->frag_no == curr_packet->total_frag) {
            printf("Completion!\n");
            close(fptr);
            free(curr_packet);
            break;
        }
        free(curr_packet);

    }

    close(socket_desc);
    return 0;
}