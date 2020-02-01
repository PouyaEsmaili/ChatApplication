#include "UI.h"
#include <stdio.h>
#include <stdlib.h>
#include "client.h"

void show_messages(Message *m, int n);
void print_message(Message m);
void reset_messages();
void show_members(Member *members, int n);

Menu account_menu = {"Account Menu", {"Register", "Login", "Exit"}, 3};
Menu main_menu = {"Main Menu", {"Create Channel", "Join Channel", "Logout"}, 3};
Menu chat_menu = {"Chat Menu", {"Send Message", "Refresh Messages", "Show All Messages", "Channel Members", "Leave Channel"}, 5};

Message *messages;
int messages_size;
int messages_last;
int messages_min_size = 1024;

int UI(char *token)
{
	messages_size = messages_min_size;
	messages_last = -1;
	messages = malloc(messages_size * sizeof(Message));

	int state = 1;
	while(state)
	{
		int res = 1;
		while(state == 1)
		{
			res = account(token);
			if(res == 0)
			{
				state = 2;
			}
			else if(res == 1)
			{
				return 0;
			}
		}

		while(res != 0 && state == 2)
		{
			res = _main(token);
			if(res == 1)
			{
				state = 1;
				break;
			}
			else if(res == 0)
			{
				state = 3;
				break;
			}
			else if(res == -2)
			{
				state == 1;
				break;
			}
		}

		while(state == 3)
		{
			res = chat(token);
			if(res == 1)
			{
				state = 2;
				break;
			}
			else if(res == -2)
			{
				state = 1;
				break;
			}
		}
	}
}

int account(char *token)
{
	char username[MAX];
	char password[MAX];
	char temp[MAX];

	int choice = menu(account_menu);
	if(choice == 1)
	{
		printf("Enter username:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", username);
		printf("Enter password:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", password);

		reg(username, password);
	}
	else if(choice == 2)
	{
		printf("Enter username:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", username);
		printf("Enter password:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", password);

		return login(username, password, token);
	}
	else if(choice == 3)
		return 1;

	return -1;
}

int _main(char *token)
{
	char name[MAX];
	char temp[MAX];

	int choice = menu(main_menu);
	if(choice == 1)
	{
		printf("Enter channel name:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", name);

		return create_channel(name, token);
	}
	else if(choice == 2)
	{
		printf("Enter channel name:");
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", name);

		return join_channel(name, token);
	}
	else if(choice == 3)
	{
		int res = logout(token);
		if(res == 0)
		{
			reset_messages();
			return 1;
		}
		return res;
	}

	return -1;
}

int chat(char *token)
{
	int choice = menu(chat_menu);
	if(choice == 1)
	{
		char message[MAX];
		char temp[MAX];
		fflush(stdout);
		fgets(temp, MAX, stdin);
		sscanf(temp, "%[^\n]s", message);

		return send_message(message, token);
	}
	else if(choice == 2)
	{
		Message *m;
		int n;
		int res = get_message(&m, &n, token);
		if(res != 0)
			return res;

		for(int i = 0; i < n; i++)
		{
			message_push_back(&messages, m[i], &messages_size, &messages_last);
		}
		if(n)
		{
			printf("New messages\n");
			show_messages(m, n);
			free(m);
		}
		else
			printf("No new messages\n");
		return 0;
	}
	else if(choice == 3)
	{
		if(messages_last < 0)
			printf("No new messages\n");
		else
			show_messages(messages, messages_last + 1);
		printf("(If you think there are new messages, refresh messages)\n");

		return 0;
	}
	else if(choice == 4)
	{
		int members_size;
		Member *members;
		int res = get_member(&members, &members_size, token);
		if(res != 0)
			return res;

		show_members(members, members_size);
		free(members);
		return 0;
	}
	else if(choice == 5)
	{
		int res = leave_channel(token);
		if(res == 0)
		{
			reset_messages();
			return 1;
		}
		return res;
	}
	return -1;
}

int menu(Menu menu)
{
	int res = 0;
	printf("--------------------------------------------------------\n");
	printf("%s\n", menu.name);
	for(int i = 0; i < menu.items_size; i++)
	{
		printf("\t%d - %s\n", i + 1, menu.items[i]);
	}
	printf("\tEnter your choice:");
	fflush(stdout);
	char temp[MAX];
	if(fgets(temp, MAX, stdin) != NULL)
	{
		temp[3] = 0;
		sscanf(temp, "%d", &res);
	}
	printf("--------------------------------------------------------\n");
	return res;
}

void show_messages(Message *m, int n)
{
	for (int i = 0; i < n; ++i)
	{
		print_message(m[i]);
	}
}

void print_message(Message m)
{
	printf("%s: %s\n", m.sender, m.content);
}

void reset_messages()
{
	if(messages != NULL)
	{
		for(int i = 0; i <= messages_last; i++)
		{
			free(messages[i].sender);
			free(messages[i].content);
		}
		free(messages);
		messages_last = -1;
		messages_size = messages_min_size;
		messages = malloc(messages_size * sizeof(Message));
	}
}

void show_members(Member *members, int n)
{
	for (int i = 0; i < n; ++i)
	{
		printf("%s", members[i].username);
		if(i < n - 1)
			printf(", ");
	}
	printf("\n");
}