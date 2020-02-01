#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H


#define SA struct sockaddr
#define MAX 1000

typedef struct User User;
typedef struct Channel Channel;
typedef struct Message Message;
typedef struct Response Response;

struct User{
	char* username;
	char* password;
	char* token;
	char* channel;
	int last_seen_message;
};

struct Channel{
	char* name;
	Message* messages;
	int messages_len;
};

struct Message{
	char* sender;
	char* content;
};

struct Response{
	char* type;
	char* content;
};

int init(int PORT);
int start();


#endif //SERVER_SERVER_H