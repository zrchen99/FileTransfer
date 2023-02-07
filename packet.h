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
    sscanf(message, "%d:%d:%d:%s:[.]", &total_frag, &frag_no, &size, filename);

    new_packet->total_frag = total_frag;
    new_packet->frag_no = frag_no;
    new_packet->size = size;
    new_packet->filename = malloc(strlen(filename) + 1);//plus one for null terminator
    strcpy(new_packet->filename, filename);
    
    // we cannot use strrchr to find the last ":" 
    // since we cannot assume the data doesn't contain any ":"
    const char *start = strchr(message, ':') + 1;
    start = strchr(start, ':') + 1;
    start = strchr(start, ':') + 1;
    start = strchr(start, ':') + 1;
    memcpy(new_packet->filedata, start, new_packet->size);
    
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