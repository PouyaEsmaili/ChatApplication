#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "UI.h"

char token[MAX];

int err_handler(Response res);

int main()
{
	int error;
	error = init(err_handler);
	if(error != 0){
		return 1;
	}

	UI(token);
	return 0;
}

int err_handler(Response res)
{
	printf("Type: %s, Content: %s\n", res.type, res.content);
	if(!strcmp(res.content, "Authentication failed!"))
		return -2;
	return -1;
}