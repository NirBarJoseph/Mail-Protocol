#include "header.h"

char* client_buff = NULL;

/*FOR WHITEBOX TESTING ONLY*/
int test_main(int argc, char* args[]){

	printf("---------------------\n"
		   "|                   |\n"
		   "|   IN TEST MODE    |\n"
		   "|                   |\n"
		   "---------------------\n");

	return 0;

}

int main(int argc, char* args[]){

	int clientSocket, maxfd ;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	fd_set read_fds;

	/*---- login variables ----*/
	bool logged_in = FALSE;
	char username[MAX_NAME_LENGTH];
	char psswrd[MAX_PASSWORD_LENGTH];
	char credentials[MAX_CREDENTIALS_LENGTH];

	/*---- Create the socket. The three arguments are: ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	/*---- Configure settings of the server address struct ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number */
	serverAddr.sin_port = argc < 3 ? htons(DEFAULT_PORT) : htons(atoi(args[2]));
	/* Set IP address */
	serverAddr.sin_addr.s_addr = argc < 2 ? inet_addr(DEFAULT_HOST) : inet_addr(args[1]);
	/* Set all bits of the padding field to 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Connect the socket to the server using the address struct ----*/
	addr_size = sizeof serverAddr;
	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

	/*---- get the greeting message and display it ----*/
	recv_with_size(clientSocket , &client_buff);
	if(client_buff){
		printf("%s\n", client_buff);
		NULLIFY(client_buff);
	} else {
		LOG_ERROR;
	}

	/*---- login section ----*/
	while(!logged_in){
		//get the name and password by the format given from the user input
		memset(credentials, 0, sizeof(credentials));
		memset(username, 0, sizeof(username));
		memset(psswrd, 0, sizeof(psswrd));
		scanf("%*s %s", username);
		scanf("%*s %s", psswrd);
		strcat(credentials, username);
		strcat(credentials, "\n");
		strcat(credentials, psswrd);

		//send it to the server
		send_with_size(clientSocket, credentials);

		/*---- get the connected/not message ----*/
		recv_with_size(clientSocket , &client_buff);
		printf("%s", client_buff);
		if (!strcmp(client_buff, CONNECTED_MSG)){
			logged_in = TRUE;
		}
	}

	maxfd = (STDIN > clientSocket) ? STDIN : clientSocket + 1;
	/*---- main logic section ----*/
	while (TRUE){

		FD_ZERO(&read_fds);
		FD_SET(clientSocket, &read_fds);
        FD_SET(STDIN, &read_fds);
//		FD_SET(STDIN_FILENO, &write_fds);

		select(maxfd, &read_fds, NULL, NULL, NULL);

		if(FD_ISSET(STDIN, &read_fds)){
			/*---- Get the request from the user and parse with the client_state_machine ----*/
			NULLIFY(client_buff);
			client_buff = get_line();
			client_state_machine(&client_buff);

			/*---- send the data and wait for response ----*/
			send_with_size(clientSocket, client_buff);

			/*---- Read the greeting message from the server and display it ----*/
			if(strstr(client_buff, "DELETE")){
				// if the cmd is delete we are not suppose to receive a msg back so continue
				continue;
			} else if(!strcmp(client_buff, "QUIT")){
				// if the cmd is quit then we need to break from the infinite loop
				// so we will close the socket and exit
				NULLIFY(client_buff);
				break;
			}
		}

		if(FD_ISSET(clientSocket, &read_fds)){
			// receive a msg back from the server and print it
			recv_with_size(clientSocket , &client_buff);
			printf("%s", client_buff);
		}
	}
	close(clientSocket);
	return 0;
}

bool client_state_machine(char** input){

	char* cmd = strtok(*input, "\n");
	char whole_msg[MAX_COMPOSE_MSG];
	char* curr_line;
	uint8 i;

	// if the command is compose we need to get more lines from the user
	if(!strcmp(cmd, "COMPOSE")){
		//append the COMPOSE string first
		curr_line = (char*)malloc(MAX_TEXT_LENGTH);
		CHECK_ALLOC(curr_line);
		memset(whole_msg, 0, MAX_COMPOSE_MSG);
		strcat(whole_msg, "COMPOSE\n");
		for (i = 0; i < 3; i++)  {
			curr_line = get_line();
			strcat(whole_msg, curr_line);
			NULLIFY(curr_line);
		}
		// remove the last '\n' from the string
		whole_msg[strlen(whole_msg)-1] = 0;
		strcpy(*input, whole_msg);
	}
	return TRUE;
}


