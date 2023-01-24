#include <its/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT     8080

int main(int argc, char *argv[]){
    int socket_desc;
    char buffer[1024];
    const char *yes = "yes";
    struct sockaddr_in servaddr, cliaddr;

    
    // Clean buffers:
    memset(&servaddr, '\0', sizeof(&servaddr));
    memset(&cliaddr, '\0', sizeof(&cliaddr));


    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    
    // Set server port and IP:
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; //not sure what this means
    servaddr.sin_port = htons(PORT);
    
    socklen_t len;
    int n;
   
    len = sizeof(cliaddr);  //len is value/result
   
    n = recvfrom(socket_desc, (char *)buffer, 1024, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);

    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    if (!strcmp(buffer, "ftp")) {
        sendto(socket_desc, (const char *)yes, strlen(yes), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
    }

    return 0;
}