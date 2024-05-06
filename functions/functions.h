#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "../user/user.h"
#include <sys/ipc.h>

#define BUFFER_LEN      200         // Buffer length.
#define INPUT_SIZE      50          // Input size.
#define MAX_FILE_LEN    20          // Max file length.
#define MSQ_QUEUE_PATH  "/tmp"      // Message queue path.
#define MAX_USERS       30          // Max users.

// Config file.
extern char* config_file;
// Message queue id.
extern int mq_id;
// Message queue key.
extern key_t key_msg;
// Shared memory id.
extern int shmid;

typedef struct{
    long priority;
    int msg;
} queue_message; // Struct to store message queue message.

/*
* Defines config_file variable to use globaly.
* @param filename File's filename.
*/
void define_config_file(char* filename);
/**
 * Verify login.
 * @param user Struct containing user login information
 * @param connection_type "UDP" or "TCP"
 * @return 1 if success, 0 otherwise
*/
int verify_login(struct_user* user, char* connection_type);
/*
* Get time in format "HH:MM:SS".
*/
char* get_time();
/*
* Returns user from file if username exists.
*/
struct_user* get_user_from_file(char* username);
/*
* Creates an array with arguments from user.
*/
char** create_args_array(int args_number);
/*
* Verify if string has numbers.
*/
int is_number(char* string);
/*
* Convert string to caps.
*/
void to_upper(char* string);
/*
* Verify if string hasn´t numbers.
*/
int verify_string(char* string);
/*
 * Invoke an error on system.
 * @param Message to describe the error.
*/
void error(const char *format, ...);
/* 
* Removes '\\n' from string.
*/
void remove_line_break(char* string);

#endif // FUNCTIONS_H