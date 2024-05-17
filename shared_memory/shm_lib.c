// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "shm_lib.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define SEM_NAME "sem"  // Semaphore name.
sem_t *sem;             // Semaphore.

/*-----------------------------------------------------------------------------------------------------*
*                                       SHARED MEMORY FUNCTIONS                                        *
*------------------------------------------------------------------------------------------------------*/

int create_shared_memory(){
    int shmid;
    key_t key;

    if((key = ftok(KEY_FILE, 'A')) == -1){
        error("[%s] LOG - create_shared_memory -> Error ftok(): ", get_time());
        kill(getpid(), SIGINT);
        kill(getppid(), SIGINT);
    }

    ssize_t size = CLASSES_SIZE * sizeof(classes_list) + MAX_USERS * sizeof(users_list);
    if((shmid = shmget(key, size, IPC_CREAT | 0700)) < 0){
        error("[%s] LOG - create_shared_memory -> Error shmget(): ", get_time());
        kill(getpid(), SIGINT);
        kill(getppid(), SIGINT);
    }

    sem_unlink(SEM_NAME);
    sem = sem_open(SEM_NAME, O_CREAT|O_EXCL, 0700, 1);
    if(sem == SEM_FAILED){
        error("[%s] LOG - create_shared_memory -> Error sem_open(): ", get_time());
        kill(getpid(), SIGINT);
        kill(getppid(), SIGINT);
    }

    printf("[%s] LOG - Shared memory created!\n", get_time());
    return shmid;
}

void* attach_shared_memory(int shmid){
    sem_wait(sem);
    void* shared_var;
	if((shared_var = shmat(shmid, NULL, 0)) == (void*) -1){
        error("[%s] LOG - attach_shared_memory -> Error shmat(): ", get_time());
        kill(getpid(), SIGINT);
        kill(getppid(), SIGINT);
	}
    return shared_var;
}

int detach_shared_memory(void* shared_var){
    sem_post(sem);
    int status = shmdt(shared_var);
    if(status == -1){
        error("[%s] LOG - detach_shared_memory -> Error shmdt(): ", get_time());
        kill(getpid(), SIGINT);
        kill(getppid(), SIGINT);
    }
    return status;
}

void remove_shared_memory(int shmid){
    sem_close(sem);
    sem_unlink(SEM_NAME);
    if(shmctl(shmid, IPC_RMID, NULL) == -1){
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}

void print_shared_memory(void* shared_var){
    classes_list* classes = (classes_list*) shared_var;
    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));

    if(classes->current_size > 0){
        printf("Classes:\n");
        for(int i = 0; i < classes->current_size; i++){
            printf("Class %d: %s\n", i, classes->classes[i].name);
            if(classes->classes[i].current_size > 0){
                for(int j = 0; j < classes->classes[i].current_size; j++){
                    printf("Student %d: %s\n", j, classes->classes[i].students[j].username);
                }
            }
        }
    } else{
        printf("No classes in shared memory.\n");
    }

    if(users->current_size > 0){
        printf("Users:\n");
        for(int i = 0; i < users->current_size; i++){
            printf("User %d: %s\n", i, users->users[i].username);
        }
    } else{
        printf("No users in shared memory.\n");
    }

    return;
}

/*-----------------------------------------------------------------------------------------------------*
*                                       CLASSES LIST FUNCTIONS                                         *
*------------------------------------------------------------------------------------------------------*/

int find_class(classes_list* classes, char* class_name){
    char* class_name_copy = to_upper(class_name);

    for(int i = 0; i < classes->current_size; i++){
        if(strcmp(to_upper(classes->classes[i].name), class_name_copy) == 0){
            return i;
        }
    }
    return -1;
}

int add_class(classes_list* classes, struct class new_class){
    if(find_class(classes, new_class.name) != -1){
        return 0;
    }

    if(classes->current_size == CLASSES_SIZE){
        return 0;
    }

    classes->classes[classes->current_size] = new_class;
    classes->current_size++;

    return 1;
}

int remove_student_from_class(classes_list* classes, char* class_name, char* username){
    int index = find_class(classes, class_name);
    if(index == -1){
        return 0;
    }

    for(int i = 0; i < classes->classes[index].current_size; i++){
        if(strcmp(classes->classes[index].students[i].username, username) == 0){
            for(int j = i; j < classes->classes[index].current_size - 1; j++){
                strcpy(classes->classes[index].students[j].username, classes->classes[index].students[j + 1].username);
                strcpy(classes->classes[index].students[j].password, classes->classes[index].students[j + 1].password);
                strcpy(classes->classes[index].students[j].type, classes->classes[index].students[j + 1].type);
            }
            classes->classes[index].current_size--;
            return 1;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------------------------------------*
*                                       USERS LIST FUNCTIONS                                           *
*------------------------------------------------------------------------------------------------------*/

int find_user(users_list* users, char* username){
    for(int i = 0; i < users->current_size; i++){
        if(strcmp(users->users[i].username, username) == 0){
            return i;
        }
    }
    return -1;
}

int remove_user(users_list* users, char* username){
    int index = find_user(users, username);
    if(index == -1){
        return 0;
    }

    for(int i = index; i < users->current_size - 1; i++){
        strcpy(users->users[i].username, users->users[i + 1].username);
        strcpy(users->users[i].password, users->users[i + 1].password);
        strcpy(users->users[i].type, users->users[i + 1].type);
    }
    users->current_size--;
    return 1;
}

int add_user(users_list* users, struct_user new_user){
    if(users->current_size == MAX_USERS){
        return 0;
    }

    users->users[users->current_size] = new_user;
    users->current_size++;
    return 1;
}

// shm_lib.c