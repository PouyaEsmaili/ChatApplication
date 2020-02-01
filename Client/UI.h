#ifndef CLIENT_UI_H
#define CLIENT_UI_H

typedef struct Menu Menu;

struct Menu
{
	char *name;
	char *items[10];
	int items_size;
};

int UI(char *token);
int account(char *token);
int _main(char *token);
int chat(char *token);
int menu(Menu menu);

#endif //CLIENT_UI_H
