#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "server.h"
#include "file.h"

char* main_address = "Resources";
char* users_address = "Resources\\Users";
char* users_format = "user.json";
char* channels_address = "Resources\\Channels";
char* channels_format = "channel.json";

void directory_init()
{
	create_directory(main_address);
	create_directory(users_address);
	create_directory(channels_address);
}

char* make_address(char* name, int type)
{
	char* address = malloc(MAX);
	memset(address, 0, MAX);

	if(type == 1)
	{
		sprintf(address, "%s\\%s.%s", users_address, name, users_format);
	}
	else if(type == 2)
	{
		sprintf(address, "%s\\%s.%s", channels_address, name, channels_format);
	}

	return address;
}

char* read_file(char* address)
{
	FILE* cfPtr = fopen(address, "r");
	if(cfPtr == NULL)
	{
		return NULL;
	}
	else
	{
		fseek(cfPtr, 0L, SEEK_END);
		int size = ftell(cfPtr) + 1;
		rewind(cfPtr);
		char buffer[MAX];
		char* result = malloc(size);
		memset(result, 0, size);
		memset(buffer, 0, MAX);
		while(!feof(cfPtr))
		{
			fgets(buffer, MAX, cfPtr);
			strcat(result, buffer);
		}
		fclose(cfPtr);
		return result;
	}
}

int write_file(char* address, char* data)
{
	FILE* cfPtr = fopen(address, "w");
	if(cfPtr == NULL)
	{
		return -1;
	}
	else
	{
		fputs(data, cfPtr);
		fclose(cfPtr);
		return 0;
	}
}

int create_directory(char* address)
{
	return mkdir(address);
}

int exist(char* address)
{
	if(access(address, F_OK ) != -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}