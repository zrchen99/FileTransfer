#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

#include "packet.h";

void send_file(char* filename, int socket_desc, struct sockaddr_in server_addr){


    FILE *fp;
    if((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        exit(1);
    }
    printf("Successfully opened file %s\n", filename);

    fseek(fp, 0, SEEK_END);

    int size = ftell(fp);

    int num_frag = size / 1000;
    if (1000 * num_frag < size) {
        num_frag++;
    }

    fseek(fp, 0, SEEK_SET);

    struct packet* fragments = (struct packet*)malloc(sizeof(struct packet) * num_frag);

    for (int i = 0; i < num_frag; i++){
        memset(fragments[i].filedata, 0, sizeof(fragments[i].filedata));
        fread(fragments[i].filedata, sizeof(char), 1000, fp);
        fragments[i].total_frag = num_frag;
        fragments[i].frag_no = i+1;
        int frag_size = 1000;
        if (i == num_frag - 1) {
            frag_size = size % 1000;
        }
        fragments[i].size = frag_size;
        fragments[i].filename = filename;

    }

    char server_message[1024];
    memset(server_message, '\0', sizeof(server_message));

    for (int i = 0; i < num_frag; i++) {
        int bytes = 0;
        char* message = packet_to_string(&fragments[i], &size);
        clock_t start = clock();
        if (sendto(socket_desc, message, bytes, 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) <= 0) {
            printf("Error while sending server msg\n");
            exit(1);
        }
        if (recvfrom(socket_desc, server_message, sizeof(server_message), 0,
         (struct sockaddr*)&server_addr, sizeof(server_addr) < 0)){
            printf("Error while receiving server's msg\n");
            exit(1);
        }
        clock_t end = clock();
        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
        printf("RTT from client to server = %f/n", seconds);
        
    }
    printf("File Transmission Finished\n");
    fclose(fp);
    free(fragments);

}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("deliver <server address> <server port number>\n");
        return -1;
    }
    int port = atoi(argv[2]);

    int socket_desc;
    struct sockaddr_in server_addr;

    char server_message[1024], client_message[1024], filename[FILENAME_MAX];
    socklen_t server_struct_length = sizeof(server_addr);

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }

    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    // Get input from the user:
    printf("Enter the message as follow: ftp filename\n");
    char command[100];
    scanf("%s", command);
    scanf("%s", filename);

    if (strcmp(command, "ftp") != 0)
    {
        printf("User input wrong format: ftp filename\n");
    }

    if (access(filename, F_OK) == -1)
    {
        printf("The file doesn't exist\n");
        return -1;
    }

    // Send the message to server:
    if(sendto(socket_desc, "ftp", strlen("ftp"), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    
    // Receive the server's response:
    int num;
    if((num = recvfrom(socket_desc, server_message, sizeof(server_message), 0,
         (struct sockaddr*)&server_addr, &server_struct_length)) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    printf("%d\n",num);
    
    if(strcmp(server_message, "yes") == 0){
        printf("Server's response: %s\n", server_message);
        printf("A file transfer can start.\n");

        send_file(filename, socket_desc, server_addr);
    }
    else{
        return -1;
    }
    // Close the socket:
    close(socket_desc);
    
    return 0;
}