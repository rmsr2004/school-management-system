#ifndef SHM_LIB_H
#define SHM_LIB_H

#include "../class/class.h"
#include "../user/user.h"

#define KEY_FILE        "/tmp"
#define CLASSES_SIZE    30
#define USERS_SIZE      30


/* ----------------------------------------*/
/*           Shared Memory Structs         */
/* ----------------------------------------*/
typedef struct{
    struct class classes[CLASSES_SIZE];     // Array of classes.
    int current_size;                       // Current size of the array.
} classes_list; // Struct to store classes in shared memory.

typedef struct{
    struct_user users[USERS_SIZE];          // Array of users.
    int current_size;                       // Current size of the array.  
} users_list; // Struct to store users logged in the system in shared memory.

/* ----------------------------------------*/
/*   Functions to handle shared memory.    */
/* ----------------------------------------*/
// Create Shared Memory.
int create_shared_memory();
// Attach shared memory.
void* attach_shared_memory(int shmid);
// Detach Shared Memory.
int detach_shared_memory(void* shared_var);
// Remove Shared Memory.
void remove_shared_memory(int shmid);


/* ----------------------------------------*/
/*       Functions to handle classes       */
/* ----------------------------------------*/

/*
* Find a class by its name.
*/
int find_class(classes_list* classes, char* class_name);

/* ----------------------------------------*/
/*       Functions to handle users         */
/* ----------------------------------------*/

/*
* Find a user by its username.
*/
int find_user(users_list* users, char* username);

#endif  // SHM_LIB_H