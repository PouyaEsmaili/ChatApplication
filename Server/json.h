#ifndef SERVER_JSON_H
#define SERVER_JSON_H

#include "server.h"

#ifdef USE_CJSON
#include "cJSON.h"
#else
typedef struct cJSON{
	char* data;
} cJSON;

char* cJSON_PrintUnformatted(cJSON* json);
void cJSON_Delete(cJSON* json);
#endif

User parse_user(char* data);
Channel parse_channel(char* data);
Message parse_message(char* data);
Message* parse_messages(char* data, int* n);
cJSON* create_user_object(User user);
cJSON* create_channel_object(Channel channel);
cJSON* create_message_list_object(Message* messages, int len);
cJSON* create_message_object(Message message);
cJSON* create_members_object(User* user, int len, char* channel_name);
cJSON* create_response_object(Response response);

#endif //SERVER_JSON_H
