#include "header.h"

int receive_all(int socketfd, char *buf, int *len) {
    int total = 0;        // how many bytes we've received
    int bytesleft = *len; // how many we have left to receive
    int n;
    char* curr_ptr = buf;

    while (total < *len) {
        n = recv(socketfd, curr_ptr, bytesleft, 0);
        if (n <= 0) {
        	break;
        }
        total += n;
        bytesleft -= n;
        curr_ptr += n;
    }

    *len = total; // return number actually received here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}


int send_all(int s, char *buf, int *len) {
	int total = 0; /* how many bytes we've sent */
	int bytesleft = *len; /* how many we have left to send */
	int n;
    char* curr_ptr = buf;
	while(total < *len) {
		n = send(s, curr_ptr, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
        curr_ptr += n;
	}
	*len = total; /* return number actually sent here */
	return n == -1 ? n:0; /*-1 on failure, 0 on success */
}

int send_with_size(int socketfd, char *msg) {
    // get size of message
    int len = strlen(msg) + 1;
    uint32 msg_size = (uint32) len;

    // put int into a 4 byte buffer
    if(send(socketfd, &msg_size, sizeof(uint32), 0) != sizeof(uint32)){
    	LOG_ERROR;
    	return -1;
    }

    // send this buffer to server

    // send the message buffer itself
    if(send_all(socketfd, msg, &len) != 0){
    	LOG_ERROR;
    	return -1;
    }
    return 0;
}

ssize_t recv_with_size(int socketfd, char **dest) {
    uint32 size_buff = 0;
    int len;

    // first, get 4 bytes representing the size of the incoming message
    memset(&size_buff, 0, sizeof(uint32));
    recv(socketfd, &size_buff, sizeof(uint32), 0);
    if(size_buff == 0){
    	printf("USER DISCONECTED UNORDERLY\n");
    	return 0;
    }

    len = (int) size_buff;

    /*receive a message with the size we got
      malloc the dest with msg_size size*/
    *dest = (char*)malloc(len);
    if(!(*dest)){
    	LOG_ERROR;
    }
    memset(*dest, 0, len);

    if (receive_all(socketfd, *dest, &len) == 0){
        return len;
    }else{
        return -1;
    }
}

char* get_line(){
    char* line = NULL, *linep = line, *linen = NULL;
    size_t lenmax = BUFFER_SIZE, len = BUFFER_SIZE;
    int c;

    line = (char*)malloc(lenmax);
    CHECK_ALLOC(line);
    linep = line;

    while(TRUE) {
        c = fgetc(stdin);
        if(c == 10 && len == BUFFER_SIZE){
        	continue;
        }
        if(c == EOF){
            break;
        }

        if(--len == 0) {
            len = lenmax;
            linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n'){
            break;
        }
    }
    *line = '\0';
    return linep;
}
