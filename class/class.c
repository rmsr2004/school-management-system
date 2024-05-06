#include "class.h"
#include "../shared_memory/shm_lib.h"
#include <string.h>

int is_full(struct class* class){
    return class->current_size >= class->size;
}

int add_student(struct class* class, struct_user student){
    struct_user new_student;
    strcpy(new_student.username, student.username);
    strcpy(new_student.password, student.password);
    strcpy(new_student.type, student.type);

    if(is_full(class)){
        return 0;
    }

    class->students[class->current_size] = new_student;
    class->current_size += 1;
    
    return 1;
}