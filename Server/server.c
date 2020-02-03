#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include "sha256.h"
#include "base64.h"
#include "file.h"
#include "json.h"
#include "server.h"
#ifdef USE_CJSON
#include "cJSON.h"
#endif

int server_socket , client_socket;
struct sockaddr_in server, client;

User *user_list;
int user_list_size;
int user_list_last;
int user_list_min_size = 1;

Response reg(char* username, char* password);
Response login(char* username, char* password);
Response logout(char* token);
Response create_channel(char* name, char* token);
Response join_channel(char* name, char* token);
Response send_message(char* content, char* token);
Response get_messages(char* token);
Response get_members(char* token);
Response leave(char* token);

int server_message(char* message, char* channel);

int make_user(char* username, char* password);
int make_channel(char* name);
User get_user(char* username);
Channel get_channel(char* name);

User* find_user_by_username(char* username);
User* find_user_by_token(char* token);

Response make_response(char* type, char* value);

void user_push_back(User user);
int user_pop(User user);

int check_password(User user, char* password);
char* create_user_hash(User user);
char* create_token(User user);

void delete_channel(Channel channel);

int init(int PORT)
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}

	//Create a socket
	if((server_socket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if(bind(server_socket, (SA*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
	}

	//Listen to incoming connections
	listen(server_socket , SOMAXCONN);

	directory_init();

	user_list = malloc(user_list_min_size * sizeof(User));
	user_list_size = user_list_min_size;
	user_list_last = -1;
}

int start()
{
	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	int len = sizeof(struct sockaddr_in);
	while((client_socket = accept(server_socket , (SA*)&client, &len)) != INVALID_SOCKET)
	{
		printf("Connection accepted");

		cJSON* res_json;
		Response response;

		char receive_buffer[1000];
		memset(receive_buffer, 0, MAX);
		char command[MAX];
		memset(command, 0, MAX);
		recv(client_socket, receive_buffer, MAX, 0);
		sscanf(receive_buffer, "%s", command);

		if(strcmp(command, "register") == 0)
		{
			char username[MAX];
			char password[MAX];
			memset(username, 0, MAX);
			memset(password, 0, MAX);

			sscanf(receive_buffer, "%*s %[^,], %[^\n]", username, password);
			response = reg(username, password);
		}
		else if(strcmp(command, "login") == 0)
		{
			char username[MAX];
			char password[MAX];
			memset(username, 0, MAX);
			memset(password, 0, MAX);

			sscanf(receive_buffer, "%*s %[^,], %[^\n]", username, password);
			response = login(username, password);
		}
		else if(strcmp(command, "logout") == 0)
		{
			char token[MAX];
			memset(token, 0, MAX);

			sscanf(receive_buffer, "%*s %[^\n]", token);
			response = logout(token);
		}
		else if(strcmp(command, "create") == 0)
		{
			sscanf(receive_buffer, "%*s %s", command);
			if(strcmp(command, "channel") == 0)
			{
				char token[MAX], name[MAX];
				memset(name, 0, MAX);
				memset(token, 0, MAX);

				sscanf(receive_buffer, "%*s %*s %[^,], %[^\n]", name, token);
				response = create_channel(name, token);
			}
			else
			{
				response = make_response("Error", "Wrong command");
			}
		}
		else if(strcmp(command, "join") == 0)
		{
			sscanf(receive_buffer, "%*s %s", command);
			if(strcmp(command, "channel") == 0)
			{
				char token[MAX], name[MAX];
				memset(name, 0, MAX);
				memset(token, 0, MAX);

				sscanf(receive_buffer, "%*s %*s %[^,], %[^\n]", name, token);
				response = join_channel(name, token);
			}
			else
			{
				response = make_response("Error", "Wrong command");
			}
		}
		else if(strcmp(command, "send") == 0)
		{
			char token[MAX], message[MAX];
			memset(message, 0, MAX);
			memset(token, 0, MAX);

			sscanf(receive_buffer, "%*s %[^,], %[^\n]", message, token);
			response = send_message(message, token);
		}
		else if(strcmp(command, "refresh") == 0)
		{
			char token[MAX];
			memset(token, 0, MAX);

			sscanf(receive_buffer, "%*s %[^\n]", token);
			response = get_messages(token);
		}
		else if(strcmp(command, "channel") == 0)
		{
			sscanf(receive_buffer, "%*s %s", command);
			if(strcmp(command, "members") == 0)
			{
				char token[MAX];
				memset(token, 0, MAX);

				sscanf(receive_buffer, "%*s %*s %[^\n]", token);
				response = get_members(token);
			}
			else
			{
				response = make_response("Error", "Wrong command");
			}
		}
		else if(strcmp(command, "leave") == 0)
		{
			char token[MAX];
			memset(token, 0, MAX);

			sscanf(receive_buffer, "%*s %[^\n]", token);
			response = leave(token);
		}
		else
		{
			response = make_response("Error", "Wrong command");
		}
		res_json = create_response_object(response);
		free(response.type);
		free(response.content);

		char* data = cJSON_PrintUnformatted(res_json);
		strcat(data, "\n");
		send(client_socket, data, strlen(data), 0);
		printf(" %s", data);
		cJSON_Delete(res_json);
		closesocket(client_socket);
	}

	return INVALID_SOCKET;
}

Response reg(char* username, char* password)
{
	User user = get_user(username);
	if(user.username == NULL && user.password == NULL)
	{
		if(make_user(username, password) == 0)
			return make_response("Successful", "");
		else
			return make_response("Error", "Could not save the user");
	}
	else
	{
		return make_response("Error", "User already exists");
	}
}

Response login(char* username, char* password)
{
	User user = get_user(username);
	User* temp_user = find_user_by_username(username);

	if(user.username == NULL && user.password == NULL)
	{
		return make_response("Error", "User does not exist");
	}
	else if(check_password(user, password))
	{
		if(temp_user != NULL)
			return make_response("Error", "User is already logged in");

		char* token = create_token(user);

		user.token = token;
		user_push_back(user);

		return make_response("AuthToken", token);
	}
	else
	{
		return make_response("Error", "Password is incorrect");
	}
}

Response logout(char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}
	else
	{
		user_pop(*user);
		return make_response("Successful", "");
	}
}

Response create_channel(char* name, char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}

	Channel channel = get_channel(name);

	if(channel.name == NULL && channel.messages == NULL && channel.messages_len == 0)
	{
		if(make_channel(name) == 0)
		{
			user->channel = malloc(strlen(name) + 1);
			strcpy(user->channel, name);

			char m[MAX];
			memset(m, 0, MAX);
			sprintf(m, "%s created %s", user->username, user->channel);
			server_message(m, user->channel);

			return make_response("Successful", "");
		}
		else
			return make_response("Error", "Could not save the channel");
	}
	else
	{
		delete_channel(channel);
		return make_response("Error", "Channel already exists");
	}
}

Response join_channel(char* name, char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}

	Channel channel = get_channel(name);

	if(channel.name == NULL && channel.messages == NULL && channel.messages_len == 0)
	{
		return make_response("Error", "There is no channel with this name");
	}

	delete_channel(channel);
	user->channel = malloc(strlen(name) + 1);
	strcpy(user->channel, name);

	char m[MAX];
	memset(m, 0, MAX);
	sprintf(m, "%s joined %s", user->username, user->channel);
	server_message(m, user->channel);

	return make_response("Successful", "");
}

Response send_message(char* content, char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}
	else if(user->channel == NULL)
	{
		return make_response("Error", "You are not joined in any channel");
	}

	Channel channel = get_channel(user->channel);

	if(channel.name == NULL && channel.messages == NULL && channel.messages_len == 0)
	{
		return make_response("Error", "Your current channel does not exist");
	}

	Message message;
	message.sender = malloc(strlen(user->username) + 1);
	message.content = malloc(strlen(content) + 1);
	strcpy(message.sender, user->username);
	strcpy(message.content, content);

	channel.messages_len++;
	channel.messages = realloc(channel.messages, channel.messages_len * sizeof(Message));
	channel.messages[channel.messages_len - 1] = message;

	cJSON* channel_json = create_channel_object(channel);

	char* address = make_address(channel.name, 2);

	if(channel_json != NULL)
	{
		int res = write_file(address, cJSON_PrintUnformatted(channel_json));
		delete_channel(channel);
		cJSON_Delete(channel_json);
		free(address);
		if(res == 0)
			return make_response("Successful", "");
		else
			return make_response("Error", "Could not save");
	}
	else
	{
		delete_channel(channel);
		free(address);
		return make_response("Error", "Could not make json object");
	}
}

int server_message(char* message, char* channel)
{
	Channel c = get_channel(channel);

	if(c.name != NULL && c.messages != NULL)
	{
		Message m;
		char* sender = "server";
		m.sender = malloc(strlen(sender) + 1);
		m.content = malloc(strlen(message) + 1);
		strcpy(m.sender, sender);
		strcpy(m.content, message);

		c.messages_len++;
		c.messages = realloc(c.messages, c.messages_len * sizeof(Message));
		c.messages[c.messages_len - 1] = m;

		cJSON* channel_json = create_channel_object(c);

		char* address = make_address(channel, 2);

		if(channel_json != NULL)
		{
			int res = write_file(address, cJSON_PrintUnformatted(channel_json));
			cJSON_Delete(channel_json);
			delete_channel(c);
			free(address);
			return res;
		}
		free(address);
	}
	return -1;
}

Response get_messages(char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}
	else if(user->channel == NULL)
	{
		return make_response("Error", "You are not joined in any channel");
	}

	Channel channel = get_channel(user->channel);

	if(channel.name == NULL && channel.messages == NULL && channel.messages_len == 0)
	{
		return make_response("Error", "Your current channel does not exist");
	}

	cJSON* messages_list_json = create_message_list_object(&channel.messages[user->last_seen_message],
			channel.messages_len - user->last_seen_message);
	delete_channel(channel);

	if(messages_list_json != NULL)
	{
		user->last_seen_message = channel.messages_len;
		Response response = make_response("List", cJSON_PrintUnformatted(messages_list_json));
		cJSON_Delete(messages_list_json);

		return response;
	}
	else
	{
		return make_response("Error", "Could not make json object");
	}
}

Response get_members(char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
		return make_response("Error", "Authentication failed!");

	cJSON* members_json = create_members_object(user_list, user_list_last + 1, user->channel);

	if(members_json == NULL)
		return make_response("Error", "Could not create list");

	Response response = make_response("List", cJSON_PrintUnformatted(members_json));

	cJSON_Delete(members_json);

	return response;
}

Response leave(char* token)
{
	User* user = find_user_by_token(token);

	if(user == NULL)
	{
		return make_response("Error", "Authentication failed!");
	}

	Channel channel = get_channel(user->channel);

	if(channel.name == NULL && channel.messages == NULL && channel.messages_len == 0)
	{
		return make_response("Error", "There is no channel with this name");
	}

	char m[MAX];
	memset(m, 0, MAX);
	sprintf(m, "%s leaved %s", user->username, user->channel);
	server_message(m, user->channel);

	free(user->channel);
	user->channel = NULL;
	user->last_seen_message = 0;

	delete_channel(channel);

	return make_response("Successful", "");
}

int make_user(char* username, char* password)
{
	char* address = make_address(username, 1);

	User user;
	user.username = username;
	user.password = password;
	user.password = create_user_hash(user);

	cJSON* user_json = create_user_object(user);

	if(user_json != NULL)
	{
		int res = write_file(address, cJSON_PrintUnformatted(user_json));
		cJSON_Delete(user_json);
		free(address);
		return res;
	}
	else
	{
		free(address);
		return -1;
	}
}

int make_channel(char* name)
{
	char* address = make_address(name, 2);

	Channel channel;
	channel.name = NULL;
	channel.messages = NULL;
	channel.messages_len = 0;
	cJSON* channel_json = create_channel_object(channel);

	if(channel_json != NULL)
	{
		int res = write_file(address, cJSON_PrintUnformatted(channel_json));
		cJSON_Delete(channel_json);
		free(address);
		return res;
	}
	else
	{
		free(address);
		return -1;
	}
}

User get_user(char* username)
{
	char* address = make_address(username, 1);

	if(exist(address))
	{
		char* buffer = read_file(address);
		User user = parse_user(buffer);

		free(buffer);
		free(address);
		return user;
	}
	else
	{
		User user;
		user.username = NULL;
		user.password = NULL;

		free(address);
		return user;
	}
}

Channel get_channel(char* name)
{
	char* address = make_address(name, 2);

	if(exist(address))
	{
		char* buffer = read_file(address);

		Channel channel = parse_channel(buffer);
		channel.name = malloc(strlen(name) + 1);
		strcpy(channel.name, name);

		free(buffer);
		free(address);
		return channel;
	}
	else
	{
		Channel channel;
		channel.name = NULL;
		channel.messages = NULL;
		channel.messages_len = 0;

		free(address);
		return channel;
	}
}

Response make_response(char* type, char* value)
{
	Response response;
	response.type = malloc(strlen(type) + 1);
	response.content = malloc(strlen(value) + 1);
	strcpy(response.type, type);
	strcpy(response.content, value);
	return response;
}

User* find_user_by_username(char* username)
{
	for(int i = 0; i <= user_list_last; i++)
	{
		if(user_list[i].username != NULL)
			if(strcmp(user_list[i].username, username) == 0)
				return &user_list[i];
	}
	return NULL;
}

User* find_user_by_token(char* token)
{
	for(int i = 0; i <= user_list_last; i++)
	{
		if(user_list[i].token != NULL)
			if(strcmp(user_list[i].token, token) == 0)
				return &user_list[i];
	}
	return NULL;
}

void user_push_back(User user)
{
	user_list_last++;
	if(user_list_last >= user_list_size)
	{
		user_list_size *= 2;
		user_list = realloc(user_list, user_list_size * sizeof(User));
	}
	user_list[user_list_last] = user;
}

int user_pop(User user)
{
	int flag = 0;
	for (int i = 0; i <= user_list_last; i++)
	{
		if (flag)
		{
			user_list[i - 1] = user_list[i];
		}

		if (strcmp(user_list[i].token, user.token) == 0)
		{
			flag = 1;
		}
	}
	if (flag)
	{
		user_list_last--;
		return 0;
	}
	else
	{
		return -1;
	}
}

int check_password(User user, char* password)
{
	char buffer[MAX];
	memset(buffer, 0, MAX);
	sprintf(buffer, "%s%s", user.username, password);

	BYTE user_hash[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, buffer, strlen(buffer));
	sha256_final(&ctx, user_hash);

	char encoded[10 * SHA256_BLOCK_SIZE];
	memset(encoded, 0, 10 * SHA256_BLOCK_SIZE);
	base64_encode(user_hash, encoded, SHA256_BLOCK_SIZE, 1);

	return strcmp(user.password, encoded) == 0;
}

char* create_user_hash(User user)
{
	char buffer[MAX];
	memset(buffer, 0, MAX);
	sprintf(buffer, "%s%s", user.username, user.password);

	BYTE user_hash[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, buffer, strlen(buffer));
	sha256_final(&ctx, user_hash);

	char *encoded = malloc(10 * SHA256_BLOCK_SIZE);
	memset(encoded, 0, 10 * SHA256_BLOCK_SIZE);
	base64_encode(user_hash, encoded, SHA256_BLOCK_SIZE, 1);

	return encoded;
}

char* create_token(User user)
{
	char buffer[MAX];
	memset(buffer, 0, MAX);
	sprintf(buffer, "%s%s%d", user.username, user.password, time(NULL));

	BYTE token_temp[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, buffer, strlen(buffer));
	sha256_final(&ctx, token_temp);

	char *token = malloc(10 * SHA256_BLOCK_SIZE);
	base64_encode(token_temp, token, SHA256_BLOCK_SIZE, 1);

	token[SHA256_BLOCK_SIZE] = 0;
	return token;
}

void delete_channel(Channel channel)
{
	if(channel.name != NULL)
		free(channel.name);
	for (int i = 0; i < channel.messages_len; ++i)
	{
		if(channel.messages[i].sender != NULL)
			free(channel.messages[i].sender);
		if(channel.messages[i].content != NULL)
			free(channel.messages[i].content);
	}
	if(channel.messages != NULL)
		free(channel.messages);
}