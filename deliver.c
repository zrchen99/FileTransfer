#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("deliver <server address> <server port number>\n");
        return -1;
    }
    int port = atoi(argv[2]);

    int socket_desc;
    struct sockaddr_in server_addr;

    char server_message[1024], client_message[1024], filename[FILENAME_MAX];
    int server_struct_length = sizeof(server_addr);

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
    fgets(client_message, sizeof(client_message), stdin);
    

    // Check the format of user input and get filename
    if(strncmp(client_message, "ftp filename", 4) != 0){
        printf("User input is in wrong format");
        return -1;
    }
    strncpy(filename, client_message+4, sizeof(client_message));

    // Check is the file exist
    FILE *file;
    if(file = fopen(filename, "r")){
        // File exist
        fclose(file);
    }
    else{
        //File doesn't exist
        printf("The file doesn't exist.\n");
        return -1;
    }

    // Send the message to server:
    if(sendto(socket_desc, "ftp", strlen("ftp"), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    
    // Receive the server's response:
    if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
         (struct sockaddr*)&server_addr, &server_struct_length) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    
    if(strcmp(server_message, "yes")){
        printf("Server's response: %s\n", server_message);
        printf("A file transfer can start.\n");
    }
    else{
        return -1;
    }
    // Close the socket:
    close(socket_desc);
    
    return 0;
}