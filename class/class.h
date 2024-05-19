// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#ifndef CLASS_H
#define CLASS_H

#include "../user/user.h"
#include "../functions/functions.h"
#include <netinet/in.h>

/*
*   Class Containing the name, size, professor, students and address of the class.
*/
struct class{
    char name[ARGS_LEN];                // name of the class
    int size;                           // maximum number of students in the class     
    int current_size;                   // current number of students in the class
    struct_user professor;              // professor of the class
    struct_user students[MAX_USERS];    // students in the class
    struct sockaddr_in addr;            // address of the class
};

/*-----------------------------------------------------------------------------------------------------*
*                                       FUNCTIONS TO MANAGE A CLASS                                    *
*------------------------------------------------------------------------------------------------------*/

/*
*   Returns 1 if the class is full, 0 otherwise.
*/
int is_full(struct class* class);
/*
*   Add a student to a class.
*   Returns 1 if the student was added, 0 otherwise.
*/
int add_student(struct class* class, struct_user student);

#endif // CLASS_H