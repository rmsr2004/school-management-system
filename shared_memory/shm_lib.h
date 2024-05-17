#ifndef SHM_LIB_H
#define SHM_LIB_H

#include "../class/class.h"
#include "../user/user.h"

#define KEY_FILE        "/tmp"

/* ----------------------------------------*/
/*           Shared Memory Structs         */
/* ----------------------------------------*/
typedef struct{
    struct class classes[CLASSES_SIZE];     // Array of classes.
    int current_size;                       // Current size of the array.
} classes_list; // Struct to store classes in shared memory.

typedef struct{
    struct_user users[MAX_USERS];          // Array of users.
    int current_size;                      // Current size of the array.  
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
// Print shared memory.
void print_shared_memory(void* shared_var);


/* ----------------------------------------*/
/*       Functions to handle classes       */
/* ----------------------------------------*/

/*
*   Find a class by its name.
*/
int find_class(classes_list* classes, char* class_name);
/*
*   Add a class to the classes list.
*/
int add_class(classes_list* classes, struct class new_class);
/*
*   Remove a student from a class.
*/
int remove_student_from_class(classes_list* classes, char* class_name, char* username);

/* ----------------------------------------*/
/*       Functions to handle users         */
/* ----------------------------------------*/

/*
*   Find a user by its username.
*/
int find_user(users_list* users, char* username);
/*
*   Remove a user from the users list.
*/
int remove_user(users_list* users, char* username);
/*
*   Add a user to the users list.
*/
int add_user(users_list* users, struct_user new_user);

#endif  // SHM_LIB_H