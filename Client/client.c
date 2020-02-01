#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include "cJSON.h"

const int alloc_offset = 1;

struct sockaddr_in server;
const char address[] = "127.0.0.1";
const int PORT = 12345;
int (*err)(Response);
int client_socket;

int init(int (*error_handler)(Response))
{
	err = error_handler;

	//Initializing Winsock library
	WORD wVersionRequested;
	WSADATA wsa;
	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsa) != 0) {
		// Tell the user that we could not find a usable Winsock DLL.
		Response res = {"WSAStartup", ""};
		(*err)(res);
		return WSAGetLastError();
	}

	// Assign IP and port
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port = htons(PORT);

	return 0;
}

int reg(char *username, char *password)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "register %s, %s\n", username, password);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	free(res.type);
	free(res.content);
	return result;
}

int login(const char *username, const char *password, char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "login %s, %s\n", username, password);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "AuthToken") == 0)
	{
		strcpy(token, res.content);
	}
	else
	{
		result = (*err)(res);
	}

	free(res.type);
	free(res.content);
	return result;
}

int logout(char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "logout %s\n", token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	else
	{
		token = NULL;
	}
	free(res.type);
	free(res.content);
	return result;
}

int create_channel(char *name, const char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "create channel %s, %s\n", name, token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	free(res.type);
	free(res.content);
	return result;
}

int join_channel(char *name, const char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "join channel %s, %s\n", name, token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	free(res.type);
	free(res.content);
	return result;
}

int send_message(char *text, const char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "send %s, %s\n", text, token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	free(res.type);
	free(res.content);
	return result;
}

int get_message(Message **messages, int *n, char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "refresh %s\n", token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "List"))
	{
		result = (*err)(res);
	}
	else
	{
		*messages = parse_message(res.content, n);
	}
	free(res.type);
	free(res.content);
	return result;
}

int get_member(Member **members, int *n, const char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "channel members %s\n", token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "List"))
	{
		result = (*err)(res);
	}
	else
	{
		*members = parse_member(res.content, n);
	}
	free(res.type);
	free(res.content);
	return result;
}

int leave_channel(char *token)
{
	Response res;

	//Creating buffer for send and receive
	char send_buffer[MAX];
	sprintf(send_buffer, "leave %s\n", token);

	res = client(send_buffer);

	//Checking the Response
	int result = 0;
	if(strcmp(res.type, "Successful"))
	{
		result = (*err)(res);
	}
	free(res.type);
	free(res.content);
	return result;
}

Response client(char *send_buffer)
{
	Response res;
	char *receive_buffer = malloc(MAX * MAX);
	memset(receive_buffer, 0, MAX * MAX);

	// Create and verify socket
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		char *stype = "Connection";
		char *scontent = "Creating socket failed.";
		res.type = malloc(strlen(stype) + alloc_offset);
		res.content = malloc(strlen(scontent) + alloc_offset);
		strcpy(res.type, stype);
		strcpy(res.content, scontent);
		return res;
	}
	//Connecting to server
	if (connect(client_socket, (SA*)&server, sizeof(server)) != 0)
	{
		char *stype = "Connection";
		char *scontent = "Connection to the server failed.";
		res.type = malloc(strlen(stype) + alloc_offset);
		res.content = malloc(strlen(scontent) + alloc_offset);
		strcpy(res.type, stype);
		strcpy(res.content, scontent);
		return res;
	}

	//Sending data and getting the result
	int err;
	char temp_buffer[MAX];
	memset(temp_buffer, 0, MAX);
	send(client_socket, send_buffer, MAX - 1, 0);
	while((err = recv(client_socket, temp_buffer, MAX - 1, 0)) > 0)
	{
		strcat(receive_buffer, temp_buffer);
		memset(temp_buffer, 0, MAX);
	}
	res = parse(receive_buffer);

	closesocket(client_socket);

	free(receive_buffer);
	return res;
}

Response parse(const char *data)
{
	Response res;

	const cJSON *type = NULL;
	const cJSON *content = NULL;
	cJSON *data_json = cJSON_Parse(data);

	if(data_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Data is NULL";
			char *scontent = "There is a problem in received data.";
			res.type = malloc(strlen(stype) + alloc_offset);
			res.content = malloc(strlen(scontent) + alloc_offset);
			strcpy(res.type, stype);
			strcpy(res.content, scontent);
			return res;
		}
	}
	type = cJSON_GetObjectItemCaseSensitive(data_json, "type");
	content = cJSON_GetObjectItemCaseSensitive(data_json, "content");


	if(cJSON_IsString(type) && type != NULL && cJSON_IsArray(content))
	{
		res.type = malloc(strlen(type->valuestring) + alloc_offset);
		strcpy(res.type, type->valuestring);

		res.content = malloc(strlen(cJSON_PrintUnformatted(content)) + alloc_offset);
		strcpy(res.content, cJSON_PrintUnformatted(content));

	}
	else if(!cJSON_IsString(content) || content == NULL)
	{
		char *stype = "Error";
		char *scontent = "Could not receive data";
		res.type = malloc(strlen(stype) + alloc_offset);
		res.content = malloc(strlen(scontent) + alloc_offset);
		strcpy(res.type, stype);
		strcpy(res.content, scontent);
	}
	else
	{
		res.type = malloc(strlen(type -> valuestring) + alloc_offset);
		res.content = malloc(strlen(content -> valuestring) + alloc_offset);
		strcpy(res.type, type -> valuestring);
		strcpy(res.content, content -> valuestring);
	}
	cJSON_Delete(data_json);
	return res;
}

Message* parse_message(const char *data, int *n)
{
	Response res;

	cJSON *messages = cJSON_Parse(data);
	cJSON *message = NULL;

	if(messages == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Messages is NULL";
			char *scontent = "There is a problem in received data.";
			res.type = malloc(strlen(stype) + alloc_offset);
			res.content = malloc(strlen(scontent) + alloc_offset);
			strcpy(res.type, stype);
			strcpy(res.content, scontent);
			(*err)(res);
			return NULL;
		}
	}

	if(cJSON_IsArray(messages))
	{
		*n = cJSON_GetArraySize(messages);
		if(*n == 0)
		{
			return NULL;
		}
		Message *result = malloc(*n * sizeof(Message));
		int i = 0;
		cJSON_ArrayForEach(message, messages)
		{
			cJSON *sender = cJSON_GetObjectItemCaseSensitive(message, "sender");
			cJSON *content = cJSON_GetObjectItemCaseSensitive(message, "content");

			if(cJSON_IsString(sender) && cJSON_IsString(content))
			{
				result[i].sender = malloc(strlen(sender -> valuestring) + alloc_offset);
				result[i].content = malloc(strlen(content -> valuestring) + alloc_offset);
				strcpy(result[i].sender, sender -> valuestring);
				strcpy(result[i].content, content -> valuestring);
			}
			else
			{
				char *stype = "Client";
				char *scontent = "Error in reading data";
				res.type = malloc(strlen(stype) + alloc_offset);
				res.content = malloc(strlen(scontent) + alloc_offset);
				strcpy(res.type, stype);
				strcpy(res.content, scontent);
			}
			i++;
		}
		cJSON_Delete(messages);
		return result;
	}
	else
	{
		cJSON_Delete(messages);
		return NULL;
	}
}

Member* parse_member(const char *data, int *n)
{
	Response res;

	cJSON *members = cJSON_Parse(data);
	cJSON *member = NULL;

	if(members == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Members is NULL";
			char *scontent = "There is a problem in received data.";
			res.type = malloc(strlen(stype) + alloc_offset);
			res.content = malloc(strlen(scontent) + alloc_offset);
			strcpy(res.type, stype);
			strcpy(res.content, scontent);
			(*err)(res);
			return NULL;
		}
	}

	if(cJSON_IsArray(members))
	{
		*n = cJSON_GetArraySize(members);
		Member *result = (Member *)malloc(*n * sizeof(Member));
		int i = 0;
		cJSON_ArrayForEach(member, members)
		{
			if(cJSON_IsString(member))
			{
				result[i].username = (char *)malloc(strlen(member -> valuestring) + alloc_offset);
				strcpy(result[i].username, member -> valuestring);
			}
			else
			{
				char *stype = "Client";
				char *scontent = "Error in reading data";
				res.type = malloc(strlen(stype) + alloc_offset);
				res.content = malloc(strlen(scontent) + alloc_offset);
				strcpy(res.type, stype);
				strcpy(res.content, scontent);
				(*err)(res);
				cJSON_Delete(members);
				return NULL;
			}
			i++;
		}
		cJSON_Delete(members);
		return result;
	}
	else
	{
		cJSON_Delete(members);
		return NULL;
	}
}

void change_error_handler(int (*error_handler)(Response))
{
	err = error_handler;
}

void message_push_back(Message **messages, Message m, int *m_size, int *m_index)
{
	*m_index += 1;

	if(*m_index >= *m_size)
	{
		*m_size *= 2;
		int len = *m_size * sizeof(Message) + alloc_offset;
		*messages = realloc(*messages, len);
		memset(*messages, 0, len);
		if(*messages == NULL)
		{
			perror("realloc of messages failed");
		}
	}
	*(*messages + *m_index) = m;
}