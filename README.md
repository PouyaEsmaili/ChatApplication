# ChatApplication
Chat application with C for FOP project

this project contains two main part:

## Client
This part has two libraries. `client.h` sends and receives, and `UI.h` is the user interface to communicate with the user.
The main function just initializes the client with the appropriate `error_handler` and calls `UI` to start the user interface.

### client.h
All the functions return `0` if the result is expected and less than `0` in case of error.
client.h contains the following functions:

 1. `int init(int (*error_handler)(Response))`
 It initializes the socket to connect to server. `error_handler` function is a function that gets a response and returns error code of that response. It is called only if the server response has an error.
	 
2. `int reg(char *username, char *password)`
This function gets a `username` and a `password`, creates a string to send to the server, gets server response, and returns an error code.

3. `int login(const char *username, const char *password, char *token)`
It gets the `username` and `password` of the user, and if server responses with `AuthToken` it puts the token in `token`. It returns an error code.

4. `int logout(char *token)`
It gets a token and sends a logout request to the server.

5. `int create_channel(char *name, const char *token)`
It gets the name of the channel and the token of the user and sends create channel request to the server.

6. `int join_channel(char *name, const char *token)`
It gets the `name` of the channel and the `token` of the user and sends join channel request to the server.

7. `int send_message(char *text, const char *token)`
It gets the `token` of the user and a `text` and sends this `text` with the `token` to the server.

8. `int get_message(Message **messages, int *n, char *token)`
It sends refresh request to the server, and if there is no error, it puts the messages in an array of messages and puts the length of this array in `n`. In case of error, `messages` is `NULL` and `n` is `0`.

9. `int get_member(Member **members, int *n, const char *token)`
It sends get member request to the server, parses the response and puts it in `members`, `n` is the length of `members`.

10. `int leave_channel(char *token)`
It sends a leave request to the server.

11. `Response client(char *send_buffer)`
This function creates a socket to the server, sends the `send_buffer` to the server, receives the server's response, parses it and puts it in a `Response` variable and returns this response.

12. `Response parse(const char const *data)`
It gets the server's response as `data` and parses it into a `Response` variable, and returns it.

13. `Message* parse_message(const char *data, int *n)`
If the response of the server to refresh request is a list, this function is called and parses the data into a `Message` array and returns it. It puts the length of its result in `n`.

14. `Member* parse_member(const char *data, int *n)`
If the response of the server to get member request is a list, this function is called. It parses that list into an array of `Member` and returns this array. It puts the length of the array in `n`.

15. `void change_error_handler(int (*error_handler)(Response))`
It just gets an `error_handler` and replaces the `error_handler` used in this library.
	*It is never used*

16. `void message_push_back(Message **messages, Message m, int *m_size, int *m_index)`
It gets a pointer to an array of `Message`, a message `m`, length of `messages` and the index of the last member of `messages` and adds `m` at the end of `messages`.

This library uses `3` structs. One for keeping the response of server (`Response`), Another one for keeping messages (`Message`), and the last one for keeping members (`Member`).

### UI.h
1. `int UI(char *token)`
It gets a `token`, this can be empty but helps to have the token if an error occurs. This function calls other functions to navigate through the menus.

2. `int account(char *token)`
It creates the account menu and uses `client.h` for different requests from the user.

3. `int _main(char *token)`
It creates the main menu.

4. `int chat(char *token)`
It creates the chat menu.

5. `int menu(Menu menu)`
 It gets a `Menu` and shows it. It returns the choice of the user in this menu.

6. `void show_messages(Message *m, int n)`
It gets an array of messages (`m`) and it's length (`n`) and shows it.

7. `void print_message(Message m)`
It gets a single message and shows it.

8. `void reset_messages()`
It resets the `messages` array in case of the user leaving the channel.

9. `void show_members(Member *members, int n)`
It gets an array of members and it's length and shows it.

It keeps the messages in `messages` and items of each menu in a variable of type `struct Menu`. 

## Server
This part gets the client's request and answers with the appropriate response. The main function here just initializes the server by calling `init` function and then calls `start` to start the process.
The server keeps data in files in two directories, `Users` and `Channels`. It has the following libraries:
### server.h
It has many functions but most of them are the same as `client.h` functions. For example, `reg` gets a `username` and a `password`, makes sure this user does not already exist and then saves the user. They return a `Response`.
I will explain the new functions here:

1. `int init(int PORT)`
It initializes the socket on the given `PORT`.

2. `int start()`
It starts accepting connection requests from the client and handles the client's request.	

3. `int server_message(char* message, char* channel)`
It adds `message` as a message with sender `server` and content `message` to the `channel`. It returns an error code.

4. `int make_user(char* username, char* password)`
It is called if the `reg` function is sure to make this user, so it adds a file to the `Users` directory for that user`.

5. `int make_channel(char* name)`
It is called if the `create_channel` function is sure to make this channel, so it adds a file to `Channels` directory for this channel.

6. `User get_user(char* username)`
If a user with this username exists, it returns the user, and if not, it returns `NULL`.

7. `Channel get_channel(char* name)`
If a channel with this name exists, it returns the channel, and if not, it returns `NULL`.

There is a list of logged-in users (`user_list`). The following 4 functions use that.

8. `User* find_user_by_username(char* username)`
It returns a pointer to the user in the `user_list` with the same username if it exists. It returns `NULL` otherwise.

9. `User* find_user_by_token(char* token)`
It returns a pointer to the user in the `user_list` with the same token if it exists. It returns `NULL` if the user does not exist.

10. `void user_push_back(User user)`
It adds `user` to the `user_list`.

11. `int user_pop(User user)`
It deletes `user` from the `user_list` when the user wants to logout.

12. `Response make_response(char* type, char* value)`
It gets a `type` and a `value` and puts it in `Response` object.

13. `int check_password(User user, char* password)`
It checks if `password` is correct for this `user`.

14. `char* create_user_hash(User user)`
It creates a hash of `user` using it's `username` and `password` to be used as the user's password.

15. `char* create_token(User user)`
It creates an `AuthToken` for the `user` and returns it.

16. `void delete_channel(Channel channel)`
It frees the memory allocated for `channel`.

It has 3 structs:

1. `Channel`
It keeps the `name` and the `messages` of a channel.
2. `Message`
It keeps a single message.
3. `Response`
It keeps the response of server to each request.
 

### json.h
It contains 2 sets of functions for creating a JSON object or parse a JSON string. The names of the functions are clear so there is no need for explanation.

### file.h
It has the following functions to work with files:

1. `void directory_init()`
It creates the directories needed to save data if they don't exist.

2. `char* make_address(char* name, int type)`
It returns the address of the file with that `name`. If `type` is `1`, it creates the address of a user file, and if it's `2`, it creates the address of a channel file.

3. `char* read_file(char* address)`
It reads all data of a file and returns it. It returns `NULL` if the file does not exist.

4. `int write_file(char* address, char* data)`
It saves the `data` into a file. It returns `0` if it is successful and `-1` in case of failure.

5. `int create_directory(char* address)`
It creates a directory using the `address`.

6. `int exist(char* address)`
It checks if the file exists. It returns `1` if the file exists and `0` if it doesn't exist.
