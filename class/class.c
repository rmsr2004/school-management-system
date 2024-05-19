// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "class.h"
#include "../shared_memory/shm_lib.h"
#include <string.h>

/*-----------------------------------------------------------------------------------------------------*
*                                       FUNCTIONS TO MANAGE A CLASS                                    *
*------------------------------------------------------------------------------------------------------*/

int is_full(struct class* class){
    return class->current_size == class->size;
}

int add_student(struct class* class, struct_user student){
    /*
    *   Check if student is already in the class.
    */

    for(int i = 0; i < class->current_size; i++){
        if(strcmp(class->students[i].username, student.username) == 0){
            return 0;
        }
    }

    if(is_full(class)){
        return 0;
    }

    class->students[class->current_size] = student;
    class->current_size += 1;
    
    return 1;
}

// class.c