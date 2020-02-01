#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "cJSON.h"
#include "json.h"

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