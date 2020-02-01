#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#define SA struct sockaddr
#define MAX 1000
typedef struct Response Response;
typedef struct Message Message;
typedef struct Member Member;

struct Response
{
	char *type;
	char *content;
};

struct Message
{
	char *sender;
	char *content;
};

struct Member
{
	char *username;
};

int init(int (*error_handler)(Response));
int reg(char *username, char *password);
int login(const char *username, const char *password, char *token);
int logout(char *token);
int create_channel(char *name, const char *token);
int join_channel(char *name, const char *token);
int send_message(char *text, const char *token);
int get_message(Message **messages, int *n, char *token);
int get_member(Member **members, int *n, const char *token);
int leave_channel(char *token);
Response client(char *send_buffer);
Response parse(const char const *data);
Message* parse_message(const char *data, int *n);
Member* parse_member(const char *data, int *n);
void change_error_handler(int (*error_handler)(Response));
void message_push_back(Message **messages, Message m, int *m_size, int *m_index);

#endif //CLIENT_CLIENT_H
