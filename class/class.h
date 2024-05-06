#ifndef CLASS_H
#define CLASS_H

#include "../user/user.h"
#include "../functions/functions.h"
#include <netinet/in.h>

/*
* Class Containing the name, size, professor, students and address of the class.
*/
struct class{
    char name[20];
    int size;
    int current_size;
    struct_user professor;
    struct_user students[MAX_USERS];
    struct sockaddr_in addr;
};

/*
* Returns 1 if the class is full, 0 otherwise.
*/
int is_full(struct class* class);
/*
* Add a student to a class.
*/
int add_student(struct class* class, struct_user student);

#endif // CLASS_H