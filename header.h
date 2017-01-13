
#ifndef HEADER_H_
#define HEADER_H_

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

typedef int bool;
#define TRUE 1
#define FALSE 0

typedef unsigned char 	BYTE;
typedef uint8_t 		uint8;
typedef uint16_t 		uint16;
typedef uint32_t 		uint32;
typedef uint64_t 		uint64;

/* String Defines */
#define WELCOME_MSG 				"Helllllllllo!!! LA LA LA!!!"
#define CONNECTED_MSG				"Connected to server\n"
#define WRONG_CREDS_MSG				"Wrong credentials\n"
#define INVALID_CMD					"Invalid Command\n"
#define ERROR_MSG					"ERROR OCCURED!\n"
#define MAIL_SENT					"Mail sent\n"
#define NO_LOGGED_USER				"No Logged User\n"

/* Constants Defines */
#define NUM_OF_CLIENTS				20
#define MAX_TEXT_LENGTH 			2048
#define MAX_SUBJECT_LENGTH  		100
#define MAX_NUM_RECIPIENT   		20
#define MAX_NAME_LENGTH     		50
#define MAX_MSG_NUM					1<<15
#define MAX_COMPOSE_MSG     		MAX_TEXT_LENGTH + MAX_SUBJECT_LENGTH + MAX_NAME_LENGTH*MAX_NUM_RECIPIENT
#define MAX_PASSWORD_LENGTH 		50
#define MAX_CREDENTIALS_LENGTH 		MAX_NAME_LENGTH + MAX_PASSWORD_LENGTH
#define MAX_USER_FILE_LINE_LENGTH 	MAX_CREDENTIALS_LENGTH + 1
#define BUFFER_SIZE					1024
#define INT_SIZE					4
#define EXTRA_ARGS					4

/* Functions Defines */
#define LOG_ERROR					printf("Critical error (%s) in file: %s, in line: %d\n",\
														strerror(errno), __FILE__, __LINE__);\
									exit(1)

#define CHECK_ALLOC(x)				if(!(x)){LOG_ERROR;}

#define ALLOC_STRING_COPY(s,s2)		(s)=(char*)malloc(strlen((s2))+1);\
									CHECK_ALLOC((s));\
									memset((s),0,strlen((s2)+1))

#define ALLOC_STRING_BY_SIZE(s,n)	(s)=(char*)malloc((n));\
									CHECK_ALLOC((s));\
									memset((s),0,n)

#define DEBUGER(n)					printf("HERE%d\n",(n))

#define SHOW_CHARS(s)				for(int iii=0;iii<strlen((s));iii++){\
										printf("|c: %c --- d: %d|\n", (s)[iii], (s)[iii]);\
									}\
									printf("-----------------\n")

#define NULLIFY(p)					if((p)!=NULL){\
										free((p));\
										(p)=NULL;\
									}

typedef enum {
	SHOW_INBOX 	= 0  ,
	GET_MAIL 	     ,
	DELETE_MAIL	     ,
	QUIT		     ,
	COMPOSE		   	 ,
	SHOW_ONLINE_USERS,
	ERROR	       	 ,
	NUM_OF_COMMANDS
} COMMAND;

#define DEFAULT_HOST            "127.0.0.1"
#define DEFAULT_PORT            6423
#define CHUNK_SIZE 				512

typedef struct mail_t{
	char 	to					[MAX_NUM_RECIPIENT][MAX_NAME_LENGTH];
	uint8	recipient_number;
	char 	subject				[MAX_SUBJECT_LENGTH];
	char 	text				[MAX_TEXT_LENGTH];
	char	sender				[MAX_NAME_LENGTH];
} mail_t;

typedef struct user_t{
	uint8	guid;
	uint8	socket_number;
	char    name			[MAX_NAME_LENGTH];
	char   	psswrd			[MAX_PASSWORD_LENGTH];
	mail_t* inbox;
	uint16	inbox_size;
} user_t;

/* server function declaretion */
bool append_user(char* line);
bool load(char* path);
void free_users();
bool init_users();
int  check_name_psswrd(char* line);
bool show_inbox();
bool send_mail(char* to, char* subject, char* text);
bool server_state_machine(char* input);
void print_mail(mail_t mail);
void parse_output(COMMAND cmd, void* args);
void parse_show_inbox();
bool delete_mail(int mail_num);
void parse_get_mail(uint8* mail_num);
void parse_show_online_inbox();

/* client function decleration */
bool client_state_machine(char** input);

/* send and recieve all funcs */
int receive_all(int socketfd, char *buf, int *len);
int send_all(int s, char *buf, int *len);
int send_with_size(int socketfd, char *msg);
ssize_t recv_with_size(int socketfd, char **dest);

char* get_line();

#endif /* HEADER_H_ */
