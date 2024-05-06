#ifndef USER_H
#define USER_H

/* Struct with info from user */
typedef struct struct_user{
    char username[15];      // User username.
    char password[15];      // User password.
    char type[20];          // User type.
} struct_user;

#endif // USER_H