#ifndef SERVER_FILE_H
#define SERVER_FILE_H

void directory_init();
char* make_address(char* name, int type); //1 for users and 2 for channels

char* read_file(char* address);
int write_file(char* address, char* data);
int create_directory(char* address);
int exist(char* address);

#endif //SERVER_FILE_H