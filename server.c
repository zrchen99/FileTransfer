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


struct packet {  
    unsigned int total_frag;  
    unsigned int frag_no; 
    unsigned int size; 
    char* filename; 
    char filedata[1000];  
} packet;

struct packet *message_to_packet(char *message){
    struct packet *new_packet = malloc(sizeof(packet));
    unsigned int total_frag, frag_no, size;
    char filename[FILENAME_MAX];
    sscanf(message, "%d:%d:%d:%[^:]", &total_frag, &frag_no, &size, filename);

    new_packet->total_frag = total_frag;
    new_packet->frag_no = frag_no;
    new_packet->size = size;
    new_packet->filename = malloc(strlen(filename) + 1);//plus one for null terminator
    strcpy(new_packet->filename, filename);
    
    const char *start = strchr(message, ':') + 1;
    start = strchr(start, ':') + 1;
    start = strchr(start, ':') + 1;
    start = strchr(start, ':') + 1;
    memcpy(packet->filedata, start, packet->size);
    
}

void create_ack_message(struct packet *curr_packet, char *server_message){
    memset(server_message, '\0', sizeof(server_message));
    strcat(server_message, curr_packet->total_frag+'0');
    strcat(server_message, ":");
    strcat(server_message, curr_packet->frag_no+'0');
    strcat(server_message, ":");
    strcat(server_message, sizeof("ACK"));
    strcat(server_message, ":");
    strcat(server_message, curr_packet->filename);
    strcat(server_message, ":");
    strcat(server_message, "ACK");
}

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
        createPacket(&curr_packet, server_message);

        // ACK sending
        if(sendto(socket_desc, server_message, strlen(server_message), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        // end condition
        if (curr_packet.frag_no == curr_packet.total_frag) {
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