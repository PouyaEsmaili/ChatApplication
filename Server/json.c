#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "json.h"

//#define USE_CJSON
#ifdef USE_CJSON
#include "cJSON.h"
#endif

#ifdef USE_CJSON
User parse_user(char* data)
{
	User user;
	cJSON *data_json = cJSON_Parse(data);
	cJSON *username = NULL;
	cJSON *password = NULL;

	if (data_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Data is NULL";
			char *scontent = "There is a problem in received data.";
			//res.type = malloc(strlen(stype) + alloc_offset);
			//res.content = malloc(strlen(scontent) + alloc_offset);
			//strcpy(res.type, stype);
			//strcpy(res.content, scontent);
			//return res;
		}
	}

	username = cJSON_GetObjectItemCaseSensitive(data_json, "username");
	password = cJSON_GetObjectItemCaseSensitive(data_json, "password");

	if (cJSON_IsString(username) && cJSON_IsString(password))
	{
		user.username = malloc(strlen(username->valuestring) + 1);
		user.password = malloc(strlen(password->valuestring) + 1);
		strcpy(user.username, username->valuestring);
		strcpy(user.password, password->valuestring);
		user.channel = NULL;
		user.token = NULL;
		user.last_seen_message = 0;
	}
	else
	{
		user.username = NULL;
		user.password = NULL;
	}
	cJSON_Delete(data_json);
	return user;
}

Channel parse_channel(char* data)
{
	Channel channel;
	cJSON *data_json = cJSON_Parse(data);
	cJSON *messages = NULL;

	if (data_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Data is NULL";
			char *scontent = "There is a problem in received data.";
			//res.type = malloc(strlen(stype) + alloc_offset);
			//res.content = malloc(strlen(scontent) + alloc_offset);
			//strcpy(res.type, stype);
			//strcpy(res.content, scontent);
			//return res;
		}
	}

	messages = cJSON_GetObjectItemCaseSensitive(data_json, "messages");

	if (cJSON_IsArray(messages))
	{
		cJSON* message = NULL;

		channel.messages_len = cJSON_GetArraySize(messages);
		channel.messages = malloc(channel.messages_len * sizeof(Message));
		memset(channel.messages, 0, channel.messages_len * sizeof(Message));

		int i = 0;
		cJSON_ArrayForEach(message, messages)
		{
			channel.messages[i] = parse_message(cJSON_PrintUnformatted(message));
			i++;
		}
	}
	else
	{
		int len = 1;
		channel.messages = malloc(len * sizeof(Message));
		memset(channel.messages, 0, len * sizeof(Message));

		char* ssender = "Message error";
		char* scontent = "Could not parse message";
		channel.messages[0].sender = malloc(strlen(ssender) + 1);
		channel.messages[0].content = malloc(strlen(scontent) + 1);
		strcpy(channel.messages[0].sender, ssender);
		strcpy(channel.messages[0].content, scontent);
	}
	cJSON_Delete(data_json);
	return channel;
}

Message parse_message(char* data)
{
	Message message;
	cJSON *data_json = cJSON_Parse(data);
	cJSON* sender = NULL;
	cJSON* content = NULL;

	if (data_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			char *stype = "Data is NULL";
			char *scontent = "There is a problem in received data.";
			//res.type = malloc(strlen(stype) + alloc_offset);
			//res.content = malloc(strlen(scontent) + alloc_offset);
			//strcpy(res.type, stype);
			//strcpy(res.content, scontent);
			//return res;
		}
	}

	sender = cJSON_GetObjectItemCaseSensitive(data_json, "sender");
	content = cJSON_GetObjectItemCaseSensitive(data_json, "content");

	if (cJSON_IsString(sender) && cJSON_IsString(content))
	{
		message.sender = malloc(strlen(sender->valuestring) + 1);
		message.content = malloc(strlen(content->valuestring) + 1);
		strcpy(message.sender, sender->valuestring);
		strcpy(message.content, content->valuestring);
	}
	else
	{
		char* ssender = "Message error";
		char* scontent = "Could not parse message";
		message.sender = malloc(strlen(ssender) + 1);
		message.content = malloc(strlen(scontent) + 1);
		strcpy(message.sender, ssender);
		strcpy(message.content, scontent);
	}

	cJSON_Delete(data_json);
	return message;
}

cJSON *create_user_object(User user)
{
	cJSON* user_json = cJSON_CreateObject();
	cJSON* username_json = cJSON_CreateString(user.username);
	cJSON* password_json = cJSON_CreateString(user.password);

	if(user_json == NULL || username_json == NULL || password_json == NULL)
		return NULL;

	cJSON_AddItemToObject(user_json, "username", username_json);
	cJSON_AddItemToObject(user_json, "password", password_json);

	return user_json;
}

cJSON* create_channel_object(Channel channel)
{
	cJSON* channel_json = cJSON_CreateObject();
	cJSON* messages = cJSON_CreateArray();

	for(int i = 0; i < channel.messages_len; i++)
	{
		cJSON* message = create_message_object(channel.messages[i]);
		cJSON_AddItemToArray(messages, message);
	}

	if(channel_json == NULL || messages == NULL)
		return NULL;

	cJSON_AddItemToObject(channel_json, "messages", messages);

	return channel_json;
}

cJSON* create_message_list_object(Message* messages, int len)
{
	cJSON* messages_list_json = cJSON_CreateArray();

	for(int i = 0; i < len; i++)
	{
		cJSON* message = create_message_object(messages[i]);
		cJSON_AddItemToArray(messages_list_json, message);
	}

	return messages_list_json;
}

cJSON* create_message_object(Message message)
{
	cJSON* messsage_json = cJSON_CreateObject();
	cJSON* sender_json = cJSON_CreateString(message.sender);
	cJSON* content_json = cJSON_CreateString(message.content);

	if(messsage_json == NULL || sender_json == NULL || content_json == NULL)
		return NULL;

	cJSON_AddItemToObject(messsage_json, "sender", sender_json);
	cJSON_AddItemToObject(messsage_json, "content", content_json);

	return messsage_json;
}

cJSON* create_members_object(User* user, int len, char* channel_name)
{
	cJSON* members_json = cJSON_CreateArray();
	if(members_json == NULL)
		return NULL;

	for (int i = 0; i < len; ++i)
	{
		if(user[i].channel == NULL)
			continue;

		if(strcmp(user[i].channel, channel_name) == 0)
		{
			cJSON* member = cJSON_CreateString(user[i].username);
			cJSON_AddItemToArray(members_json, member);
		}
	}

	return members_json;
}

cJSON* create_response_object(Response response)
{
	cJSON* response_json = cJSON_CreateObject();
	cJSON* type_json = cJSON_CreateString(response.type);
	cJSON* content_json = cJSON_CreateString(response.content);

	if(response_json == NULL || type_json == NULL || content_json == NULL)
		return NULL;
	cJSON_AddItemToObject(response_json, "type", type_json);
	cJSON_AddItemToObject(response_json, "content", content_json);

	return response_json;
}
#endif

#ifndef USE_CJSON

#include <stdio.h>

char* replace(const char* data, char* str1, char* str2);

User parse_user(char* data)
{
	char username[MAX], password[MAX];
	int uindex = 0;
	int pindex = 0;
	User user;
	int index = 13;
	while(!(data[index] == '\"' && data[index - 1] != '\\'))
	{
		username[uindex] = data[index];
		uindex++;
		index++;
	}
	index += 14;
	while(!(data[index] == '\"' && data[index - 1] != '\\'))
	{
		password[pindex] = data[index];
		pindex++;
		index++;
	}
	username[uindex] = 0;
	password[pindex] = 0;

	user.username = malloc(strlen(username) + 1);
	user.password = malloc(strlen(password) + 1);
	strcpy(user.username, username);
	strcpy(user.password, password);
	user.channel = NULL;
	user.token = NULL;
	user.last_seen_message = 0;

	return user;
}

Channel parse_channel(char* data)
{
	int size = strlen(data);
	Channel channel;

	data[size - 1] = 0;
	int n;
	channel.messages = parse_messages(&data[12], &n);
	channel.messages_len = n;

	return channel;
}

Message* parse_messages(char *data, int* n)
{
	*n = 0;
	for(int i = 0; data[i]; i++)
	{
		if(data[i] == '{')
			*n += 1;
	}

	Message* messages = malloc(*n * sizeof(Message));

	int index = 12;
	for(int i = 0; i < *n; i++)
	{
		char sender[MAX], content[MAX];
		int sindex = 0;
		int cindex = 0;
		while(!(data[index] == '\"' && data[index - 1] != '\\'))
		{
			sender[sindex] = data[index];
			sindex++;
			index++;
		}
		index += 13;
		while(!(data[index] == '\"' && data[index - 1] != '\\'))
		{
			content[cindex] = data[index];
			cindex++;
			index++;
		}
		sender[sindex] = 0;
		content[cindex] = 0;

		index += 14;

		messages[i].sender = malloc(strlen(sender) + 1);
		messages[i].content = malloc(strlen(content) + 1);
		strcpy(messages[i].sender, sender);
		strcpy(messages[i].content, content);
	}

	return messages;
}

cJSON* create_user_object(User user)
{
	char temp[MAX];
	memset(temp, 0, MAX);
	sprintf(temp, "{\"username\":\"%s\",\"password\":\"%s\"}", user.username, user.password);
	cJSON* json = malloc(sizeof(cJSON));
	json->data = malloc(strlen(temp) + 1);
	strcpy(json->data, temp);
	return json;
}

cJSON* create_channel_object(Channel channel)
{
	cJSON* json = malloc(sizeof(cJSON));

	cJSON* messages = create_message_list_object(channel.messages, channel.messages_len);
	json->data = malloc(strlen(messages->data) + 20);
	memset(json->data, 0, strlen(messages->data) + 20);
	sprintf(json->data, "{\"messages\":%s}", messages->data);
	free(messages->data);
	return json;
}

cJSON* create_message_list_object(Message* messages, int len)
{
	cJSON* json = malloc(sizeof(cJSON));

	json->data = malloc(MAX);
	memset(json->data, 0, MAX);
	strcat(json->data, "[");
	int datasize = MAX;
	int datataken = 1;

	for(int i = 0; i < len; i++)
	{
		cJSON* message = create_message_object(messages[i]);
		datataken += strlen(message->data) + 2;
		if(datataken > datasize - 100)
		{
			datasize *= 2;
			json->data = realloc(json->data, datasize);
		}
		strcat(json->data, message->data);
		if(i + 1 < len)
			strcat(json->data, ",");
		free(message->data);
	}
	strcat(json->data, "]");

	return json;
}

cJSON* create_message_object(Message message)
{
	char temp[MAX];
	memset(temp, 0, MAX);
	char* sender = replace(message.sender, "\"", "\\\"");
	char* content = replace(message.content, "\"", "\\\"");
	sprintf(temp, "{\"sender\":\"%s\",\"content\":\"%s\"}", sender, content);
	free(sender);
	free(content);
	cJSON* json = malloc(sizeof(cJSON));
	json->data = malloc(strlen(temp) + 1);
	strcpy(json->data, temp);
	return json;
}

cJSON* create_members_object(User* user, int len, char* channel_name)
{
	cJSON* json = malloc(sizeof(cJSON));

	json->data = malloc(MAX);
	memset(json->data, 0, MAX);
	strcat(json->data, "[");
	int datasize = MAX;
	int datataken = 1;

	for(int i = 0; i < len; i++)
	{
		if(user->channel == NULL)
			continue;

		if(strcmp(user[i].channel, channel_name) == 0)
		{
			strcat(json->data, "\"");
			datataken += strlen(user[i].username + 3);
			if (datataken > datasize - 100)
			{
				datasize *= 2;
				json->data = realloc(json->data, datasize);
			}
			strcat(json->data, user[i].username);
			strcat(json->data, "\"");
			strcat(json->data, ",");
		}
	}
	json->data[strlen(json->data) - 1] = 0;
	strcat(json->data, "]");

	return json;
}

cJSON* create_response_object(Response response)
{
	cJSON* json = malloc(sizeof(cJSON));
	int datasize = strlen(response.type) + strlen(response.content) + 100;
	json->data = malloc(datasize);
	memset(json->data, 0, datasize);
	if(response.content[0] != '[')
		sprintf(json->data, "{\"type\":\"%s\",\"content\":\"%s\"}", response.type, response.content);
	else
		sprintf(json->data, "{\"type\":\"%s\",\"content\":%s}", response.type, response.content);
	return json;
}

char* replace(const char* data, char* str1, char* str2)
{

	char *result;
	int i, cnt = 0;
	int newWlen = strlen(str2);
	int oldWlen = strlen(str1);

	// Counting the number of times old word
	// occur in the string
	for (i = 0; data[i] != '\0'; i++)
	{
		if (strstr(&data[i], str1) == &data[i])
		{
			cnt++;

			// Jumping to index after the old word.
			i += oldWlen - 1;
		}
	}

	// Making new string of enough length
	result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

	i = 0;
	while (*data)
	{
		// compare the substring with the result
		if (strstr(data, str1) == data)
		{
			strcpy(&result[i], str2);
			i += newWlen;
			data += oldWlen;
		}
		else
			result[i++] = *data++;
	}

	result[i] = '\0';
	return result;
}

char* cJSON_PrintUnformatted(cJSON* json)
{
	return json->data;
}

void cJSON_Delete(cJSON* json)
{
	free(json->data);
}
#endif