#include "header.h"

user_t 	users[NUM_OF_CLIENTS];
user_t* current_user = NULL;
int		online_users[NUM_OF_CLIENTS] = {0};
int		waiting_connection[NUM_OF_CLIENTS] = {0};
uint8   num_of_users = 0;
uint8	num_of_connected_clients = 0;
char* 	server_buff		/*= NULL*/;


/*FOR WHITEBOX TESTING ONLY*/
int test_main(int argc, char* args[]){

	printf("---------------------\n"
			"|                   |\n"
			"|   IN TEST MODE    |\n"
			"|                   |\n"
			"---------------------\n");

	init_users();
	load(args[1]);





	return 0;

}

int main(int argc, char* args[]){

	if(argc == 5){
		return test_main(argc, args);
	}

	int welcome_socket, newSocket, found, max_fd, num_read_ready, num_write_ready;
	uint8 i, num_waiting_conn = 0;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;
	bool logged;
	char* welcome_msg = WELCOME_MSG;
	char* connected_msg = CONNECTED_MSG;
	char* wrong_cards_msg = WRONG_CREDS_MSG;
	fd_set read_fds, write_fds, active_fds;


	if(argc < 2){
		return 1;
	}

	if(!init_users() || !load(args[1])){
		LOG_ERROR;
	}

	/*---- Create the socket. The three arguments are: ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
	welcome_socket = socket(PF_INET, SOCK_STREAM, 0);
	/*---- Configure settings of the server address struct ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number, using htons function to use proper byte order */
	//	  serverAddr.sin_port = argc == 2 ? htons(atoi(args[1])) : htons(DEFAULT_PORT);
	serverAddr.sin_port = htons(DEFAULT_PORT);
	/* Set IP address to localhost */
	serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	/* Set all bits of the padding field to 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Bind the address struct to the socket ----*/
	bind(welcome_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*---- Listen on the socket, with 5 max connection requests queued ----*/
	if(listen(welcome_socket,5)==0){
		printf("Listening\n...\n");
	} else {
		LOG_ERROR;
	}

	FD_ZERO(active_fds);
	FD_SET(welcome_socket, active_fds);
	max_fd = welcome_socket;

	while(TRUE){

		addr_size = sizeof(clientAddr);
		/*---- Accept new user ----*/
		printf("before connection!\n");
		newSocket = accept(welcome_socket, (struct sockaddr *)&clientAddr, &addr_size);
		FD_SET(newSocket, active_fds);
		if(newSocket > max_fd){
			max_fd = newSocket;
		}
		printf("got a connection!\n");

		/*---- Send the greeting message ----*/
		send_with_size(newSocket, welcome_msg);

		/*---- the cradentials loop ----*/
		logged = FALSE;
		while (!logged){
			/*---- get credentials and validate them ----*/
			recv_with_size(newSocket, &server_buff);
			found = check_name_psswrd(server_buff);
			if (found != -1){
				// if log in request confirmed we want to issue a "connected" msg
				current_user = &users[found];
				online_users[found] = newSocket;
				send_with_size(newSocket, connected_msg);
				NULLIFY(server_buff);
				logged = TRUE;
				num_of_connected_clients++;
			} else {
				// if log in request not confirmed we want to issue a "not connected" msg
				send_with_size(newSocket, wrong_cards_msg);
				NULLIFY(server_buff);
			}
		}

		/*--- the send-receive loop ----*/
		do{
			read_fds = active_fds;

	        num_read_ready = Select(max_fd + 1, &read_fds, NULL, NULL, NULL);

	        if (FD_ISSET(welcome_socket, &read_fds)) {    /* new client connection */
	    		addr_size = sizeof(clientAddr);
	    		/*---- Accept new user ----*/
	    		printf("before connection!\n");
	    		newSocket = accept(welcome_socket, (struct sockaddr *)&clientAddr, &addr_size);
	    		FD_SET(newSocket, active_fds);
	    		if(newSocket > max_fd){
	    			max_fd = newSocket;
	    		}
	    		printf("got a connection!\n");

	            for (i = 0; i < NUM_OF_CLIENTS; i++)
	                if (waiting_connection[i] < 0) {
	                	waiting_connection[i] = newSocket; /* save descriptor */
	                    break;
	                }

	            FD_SET(newSocket, &active_fds);    /* add new descriptor to set */
	            if (newSocket > max_fd)
	                max_fd = newSocket;         /* for select */

	            // one more waiting to connect and less read ready socket
	            num_waiting_conn++;
	            num_read_ready--;
	        }

	        if(num_read_ready){
	        	// check if sign in waiting sockets are ready
	        	if(num_waiting_conn){
	        		for (i = 0; i < num_waiting_conn; i++){
	        			if(FD_ISSET(waiting_connection[i], &read_fds)){
	        				/*---- get credentials and validate them ----*/
	        				recv_with_size(waiting_connection[i], &server_buff);
	        				found = check_name_psswrd(server_buff);
	        				if (found != -1){
	        					// if log in request confirmed we want to issue a "connected" msg
	        					current_user = &users[found];
	        					online_users[found] = waiting_connection[i];
	        					num_waiting_conn--;
	        					///////////////////////////////TODO
	        					send_with_size(newSocket, connected_msg);
	        					//////////////////////////////TODO
	        					logged = TRUE;
	        					num_of_connected_clients++;
	        				}/* else {//TODO
	        					// if log in request not confirmed we want to issue a "not connected" msg
	        					send_with_size(newSocket, wrong_cards_msg);
	        					NULLIFY(server_buff);
	        				}*///TODO
	        				// one less read ready socket
	        	            num_read_ready--;
        					NULLIFY(server_buff);
	        			}
	        		}
	        	}

	        	// check if there are any more read ready sockets
	        	if(!num_read_ready){
	        		break;
	        	}

	        	// if there are more read ready sockects get the data from them

	        	// check if signed in sockets are ready
				for (i = 0; i < num_of_users; i++) {
					if (0) {

					}

					switch (recv_with_size(newSocket, &server_buff)) {
					case 0:
						ALLOC_STRING_COPY(server_buff, "QUIT")
						;
						strcpy(server_buff, "QUIT");
						break;
					case -1:
						printf("RECIVE FAILED\n");
						continue;
					default:
						break;
					}

					server_state_machine(server_buff);
					if (server_buff) {
						// if there is no msg back to the user continue and don't try to send an empty msg
						send_with_size(newSocket, server_buff);
						NULLIFY(server_buff);
					}

				}
			}
			write_fds = active_fds;

	        nready = Select(maxfd+1, &rset, NULL, NULL, NULL);
		} while (num_of_connected_clients);
	}
//	free(welcome_str);
	return 0;
}

/*
 * given a line from the users file, initialize & appends the user to the users list
 */
bool append_user(char* line){

	char* name, *psswrd;
	name   = strtok(line, "\t");
	psswrd = strtok(NULL, "\t");

	if(!name || !psswrd){
		return FALSE;
	}

	memset(users[num_of_users].name, '\0', MAX_NAME_LENGTH);
	memset(users[num_of_users].psswrd, '\0', MAX_PASSWORD_LENGTH);

	if(!strcpy(users[num_of_users].name, name)){
		return FALSE;
	}

	if(!strcpy(users[num_of_users].psswrd, psswrd)){
		return FALSE;
	}

	users[num_of_users].inbox_size = 0;

	users[num_of_users].guid = num_of_users;

	num_of_users++;

	return TRUE;
}

/*
 * reading the users file and sending each line to the append_user function
 */
bool load(char* path){
	FILE* fp = NULL;
	char line[MAX_USER_FILE_LINE_LENGTH];
	char* new_line;

	fp = fopen(path, "r");
	if(!fp) {
		printf("%s\n", path);
		perror("Could not find or open the file");
		return FALSE;
	}
	while(!feof(fp)){
		memset(line, 0, MAX_USER_FILE_LINE_LENGTH);
		fgets(line,MAX_USER_FILE_LINE_LENGTH, fp);
		new_line = strtok(line, "\n");
		new_line = strtok(line, "\r");
		append_user(new_line);
	}
	return TRUE;
}



void free_users(){

//	free(users);
	return;
}

bool init_users(){
	return TRUE;
}

/*
 * parse a given line to name and password and checks the DB for the user
 */
int check_name_psswrd(char* line){

	char* name, *psswrd;
	uint16 i;
	bool match;
	//split to two lines
	name = strtok(line, "\n");
	psswrd = strtok(NULL, "\n");
	//if one parameter is missing return error value
	if(!name || !psswrd){
		return -1;
	}

	if(!name || !psswrd){
		return -1;
	}

	for(i = 0; i < num_of_users; i++){
		// if users[i] password and name match the given password and name - return TRUE
		match = (!strcmp(users[i].name, name)) && (!strcmp(users[i].psswrd, psswrd));
		if(match){
			return i;
		}
	}
	// else return error value
	return -1;
}

/*
 * testing function to print the user inbox
 */
bool show_inbox(){

	uint8 i = 0;

	if(!current_user){
		printf("No logged user\n");
		return FALSE;
	}

	for (; i < current_user->inbox_size; i++){
		print_mail(current_user->inbox[i]);
	}

	return TRUE;
}

/*
 * given a pointer to existing user and a received mail - adds the mail to the user inbox
 */
bool cpy_mail_to_inbox(user_t* user, mail_t mail){

	uint16 new_place = user->inbox_size++; // the inbox size has grown
	uint8 i;

	// enlarge the inbox
	user->inbox = (mail_t*)realloc(user->inbox, user->inbox_size*sizeof(mail_t));
	CHECK_ALLOC(user->inbox);
	//create the mail in the inbox
	strcpy(user->inbox[new_place].subject, mail.subject);
	strcpy(user->inbox[new_place].text, mail.text);
	for(i = 0; i < mail.recipient_number; i++){
		strcpy(user->inbox[new_place].to[i], mail.to[i]);
	}
	user->inbox[new_place].recipient_number = mail.recipient_number;
	strcpy(user->inbox[new_place].sender, mail.sender);
	return TRUE;
}

/*
 * given a mail parameters, create a mail and serach for the the people who should receive it
 */
bool send_mail(char* to, char* subject, char* text){
	uint16 i, user_id;
	uint8 num_recipients = 0;
	char* tmp_user_name;
	user_t* tmp_user;
	mail_t tmp_mail;

	// if 1 parameter is missing - return FALSE
	if(!to || !subject || !text){
		return FALSE;
	}
	// start creating the new mail
	strcpy(tmp_mail.subject, subject);
	strcpy(tmp_mail.text, text);

	// parse the "to" string to recipients and create a recipients list
	tmp_user_name = strtok(to, ",");
	while(tmp_user_name){//tmp_user_name){
		strcpy(tmp_mail.to[num_recipients++], tmp_user_name);
		tmp_user_name = strtok(NULL, ",");
	}
	tmp_mail.recipient_number = num_recipients;
	strcpy(tmp_mail.sender, current_user->name);

	// iterate over the recipients list
	for(i = 0; i < num_recipients; i++){
		//for each 1 - look if he is a real user
		for(user_id = 0; user_id < num_of_users; user_id++){
			if(strcmp(users[user_id].name, tmp_mail.to[i])){
				//recipient didn't match user
				continue;
			}
			// if match - append the mail to the user inbox
			tmp_user = &(users[user_id]);
			cpy_mail_to_inbox(tmp_user, tmp_mail);
		}
	}

	return TRUE;
}


/*
 * buffer should be zeroed before calling this function
 */
void parse_mail_show_inbox(mail_t mail, char* buffer, uint8 mail_id){

	sprintf(buffer, "%d %s %s\n", mail_id + 1, mail.sender, mail.subject);

}

/*
 * Inner testing function
 */
void print_mail(mail_t mail){

	uint8 i;

	printf("To: %s", mail.to[0]);
	//	printf("num of rec: %d", mail.recipient_number);
	if (mail.recipient_number > 1){
		for(i = 1; i < mail.recipient_number; i++){
			printf(",%s", mail.to[i]);
		}
	}
	printf("\nSubject: %s\nText: %s\n", mail.subject, mail.text);
}

/*
 * creating the show inbox string and copying it to the server msg buffer
 */
void parse_show_inbox(){

	uint32 curr_output_len = 0;
	uint32 curr_buffer_len = 0;
	uint8 i = 0;
	uint8 max_tmp_len = MAX_NAME_LENGTH + MAX_SUBJECT_LENGTH;
	char tmp_str[max_tmp_len];
	char* tmp_buff = NULL;


	if(!current_user){
		ALLOC_STRING_COPY(server_buff,"No Logged User\n");
		strcpy(server_buff, "No Logged User\n");
		return;
	} else if (!current_user->inbox_size){
		ALLOC_STRING_COPY(server_buff,"Empty Inbox\n");
		strcpy(server_buff, "Empty Inbox\n");
		return;
	}

	//start to create the show inbox string
	tmp_buff = (char*)malloc(BUFFER_SIZE);
	CHECK_ALLOC(tmp_buff);
	memset(tmp_buff, 0, BUFFER_SIZE);
	curr_buffer_len += BUFFER_SIZE;

	//iterate over the inbox
	while(i < current_user->inbox_size){

		//if the message is deleted (all fields are zeroed (no mail with zero recipients)) continue
		if(!current_user->inbox[i].recipient_number){
			i++;
			continue;
		}
		//create a string for one msg
		memset(tmp_str, 0 ,max_tmp_len);
		parse_mail_show_inbox(current_user->inbox[i], tmp_str, i);
		//if the the function string buffer won't be enough - reallocate it
		if(curr_buffer_len < curr_output_len + strlen(tmp_str)){
			curr_buffer_len += BUFFER_SIZE;
			tmp_buff = (char*)realloc(tmp_buff, curr_buffer_len);
			CHECK_ALLOC(tmp_buff);
		}
		//copy the 1 msg string to the function buffer
		strcat(tmp_buff, tmp_str);
		curr_output_len += strlen(tmp_str);
		i++;
	}
	//check if the inbox was actually not empty
	if(!strlen(tmp_buff)){
		NULLIFY(tmp_buff);
		ALLOC_STRING_COPY(server_buff,"Empty Inbox\n");
		strcpy(server_buff, "Empty Inbox\n");
		return;
	}
	//copy the function buffer to the server buffer
	ALLOC_STRING_COPY(server_buff,tmp_buff);
	strcpy(server_buff, tmp_buff);
	NULLIFY(tmp_buff);

}

/*
 * creates a string in the format for get_mail command and copies it to the server buffer
 */
void parse_get_mail(uint8* mail_num){

	//change the mail num to currect one (array counting)
	mail_t* mail;
	uint8	i;
	(*mail_num)--;

	if(!current_user){
		ALLOC_STRING_COPY(server_buff,"No Logged User\n");
		strcpy(server_buff, "No Logged User\n");
		return;
		// if the mail number requested is bigger then the size of the inbox then it is not existing
	} else if (current_user->inbox_size <= *mail_num){
		ALLOC_STRING_COPY(server_buff,"No such mail\n");
		strcpy(server_buff, "No such mail\n");
		return;
		//if the message is deleted (all fields are zeroed (no mail with zero recipients)) continue
	} else if (!current_user->inbox[*mail_num].recipient_number){
		ALLOC_STRING_COPY(server_buff,"This mail is deleted\n");
		strcpy(server_buff, "This mail is deleted\n");
		return;
	}

	server_buff = (char*)malloc(MAX_COMPOSE_MSG);
	CHECK_ALLOC(server_buff);
	memset(server_buff, 0, MAX_COMPOSE_MSG);

	mail = &current_user->inbox[*mail_num];

	strcat(server_buff, "To: ");
	strcat(server_buff, mail->to[0]);
	if (mail->recipient_number > 1){
		for(i = 1; i < mail->recipient_number; i++){
			strcat(server_buff,",");
			strcat(server_buff, mail->to[i]);
		}
	}
	strcat(server_buff, "\nSubject: ");
	strcat(server_buff, mail->subject);
	strcat(server_buff, "\nText: ");
	strcat(server_buff, mail->text);
	strcat(server_buff, "\n");
}


void parse_show_online_inbox(){

}

/*
 * a parse state machine to redirect the msg parsing flow by command
 */
void parse_output(COMMAND cmd, void* args){

	if(server_buff){
		NULLIFY(server_buff);
	}

	if(!current_user && cmd != QUIT){
		//if quit was called - by this point there is no current user
		//but we still want to delete the server buffer string
		ALLOC_STRING_COPY(server_buff,NO_LOGGED_USER);
		strcpy(server_buff, NO_LOGGED_USER);
		return;
	}

	switch(cmd){
	case SHOW_INBOX:
		parse_show_inbox();
		break;
	case GET_MAIL:
		parse_get_mail((uint8*)args);
		break;
	case DELETE_MAIL:
	case QUIT:
		break;
	case COMPOSE:
		ALLOC_STRING_COPY(server_buff,MAIL_SENT);
		strcpy(server_buff,MAIL_SENT);
		break;
	case SHOW_ONLINE_USERS:
		parse_show_online_inbox();
		break;
	case ERROR:
	case NUM_OF_COMMANDS:
		ALLOC_STRING_COPY(server_buff,INVALID_CMD);
		strcpy(server_buff,INVALID_CMD);
	}
}

/*
 * this function zeroing the mail fields (no real delete) in the given place
 */
bool delete_mail(int mail_num){

	uint8 i = 0;
	mail_num--;

	if(!current_user){
		return FALSE;
	}

	if(current_user->inbox_size <= mail_num){
		return FALSE;
	}
	memset(current_user->inbox[mail_num].text, 0, MAX_TEXT_LENGTH);
	memset(current_user->inbox[mail_num].subject, 0, MAX_SUBJECT_LENGTH);
	memset(current_user->inbox[mail_num].sender, 0, MAX_NAME_LENGTH);
	for (; i < current_user->inbox[mail_num].recipient_number; i++){
		memset(current_user->inbox[mail_num].to[i], 0, MAX_NAME_LENGTH);
	}
	current_user->inbox[mail_num].recipient_number = 0;

	return TRUE;
}

/*
 * the server parse command and state machine
 */
bool server_state_machine(char* input){

	char* cmd, *args, *to, *subject, *text, *garbage = NULL;
	uint8 num_arg;

	cmd = strtok(input, "\n");
	if(strstr(cmd, "GET_MAIL") || strstr(cmd, "DELETE_MAIL")){
		cmd = strtok(cmd, " ");
		args = strtok(NULL, "");
	} else {
		args = strtok(NULL, "");
		cmd = strtok(cmd, " ");
	}

	if(!strcmp(cmd, "SHOW_INBOX")){
		parse_output(SHOW_INBOX, NULL);
	} else if(!strcmp(cmd, "GET_MAIL")){
		if(!args){
			parse_output(ERROR, NULL);
			return FALSE;
		}
		num_arg = (uint8) strtol(args, &garbage, 0);
		parse_output(GET_MAIL, &num_arg);
	} else if(!strcmp(cmd, "DELETE_MAIL")){
		if(!args){
			parse_output(ERROR, NULL);
			return FALSE;
		}
		num_arg = (uint8) strtol(args, &garbage, 0);
		parse_output((delete_mail(num_arg)) ? DELETE_MAIL : ERROR, NULL);
	} else if(!strcmp(cmd, "QUIT")){
		online_users[current_user->guid] = -1;
		current_user = NULL;
		num_of_connected_clients--;
		parse_output(QUIT, NULL);
	} else if(!strcmp(cmd, "COMPOSE")){
		strtok(args, " ");
		to = strtok(NULL, "\n");
		strtok(NULL, " ");
		subject = strtok(NULL, "\n");
		strtok(NULL, " ");
		text = strtok(NULL, "\n");
		if(!send_mail(to, subject, text)){
			parse_output(ERROR, NULL);
		}
		parse_output(COMPOSE, NULL);
	} else if(!strcmp(cmd, "SHOW_ONLINE_USERS")){

	} else {
		parse_output(ERROR, NULL);
		return FALSE;
	}
	return TRUE;

}

