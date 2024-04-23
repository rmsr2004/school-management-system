#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "../user/user.h"
#include <sys/ipc.h>

#define BUFFER_LEN      1024
#define INPUT_SIZE      100
#define MAX_FILE_LEN    100
#define MSQ_QUEUE_PATH  "/tmp"

// Config file.
extern char config_file[10];
// Message queue id.
extern int mq_id;
// Message queue key.
extern key_t key;
// Struct to store message queue message.
typedef struct{
    long priority;
    int msg;
} queue_message;

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
 * Invoke an error on system.
 * @param Message to describe the error.
*/
void error(const char *format, ...);
/* 
* Removes '\\n' from string.
*/
void remove_line_break(char* string);

#endif