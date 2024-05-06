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

#define SEM_NAME "/tmp"

sem_t *sem;

int create_shared_memory(){
    int shmid;
    key_t key;

    if((key = ftok(KEY_FILE, 'A')) == -1){
        perror("Shared Memory ftok():");
        exit(EXIT_FAILURE);
    }

    if((shmid = shmget(key, (CLASSES_SIZE * sizeof(classes_list) + USERS_SIZE * sizeof(users_list)), IPC_CREAT | 0700)) < 0){
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    sem = sem_open(SEM_NAME, O_CREAT|O_EXCL, 0700, 1);
    if(sem == SEM_FAILED){
        error("sem_open: ");
    }
    return shmid;
}

void* attach_shared_memory(int shmid){
    sem_wait(sem);
    void* shared_var;
	if((shared_var = shmat(shmid, NULL, 0)) == (void*) -1){
		perror("Shmat error!");
		exit(1);
	}
    return shared_var;
}

int detach_shared_memory(void* shared_var){
    sem_post(sem);
    int status = shmdt(shared_var);
    if(status == -1){
        perror("shmdt");
        exit(EXIT_FAILURE);
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

int find_class(classes_list* classes, char* class_name){
    for(int i = 0; i < classes->current_size; i++){
        if(strcmp(classes->classes[i].name, class_name) == 0){
            return i;
        }
    }
    return -1;
}

int find_user(users_list* users, char* username){
    for(int i = 0; i < users->current_size; i++){
        if(strcmp(users->users[i].username, username) == 0){
            return i;
        }
    }
    return -1;
}