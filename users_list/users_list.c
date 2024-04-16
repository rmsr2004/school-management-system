#include "users_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

users_node* create_users_list(){
    users_node* list = (users_node*) malloc(sizeof(users_node));
    if(list == NULL){
        return NULL;
    }

    struct_user user = {0};
    list->user = user;
    list->next = NULL;

    return list;
}

int is_users_list_empty(users_node* list){
    if(list->next == NULL)
        return 1;
    return 0;
}

void destroy_users_list(users_node* list){
    users_node* aux = list;
    while(!is_users_list_empty(list)){
        aux = list;
        list = list->next;
        free(aux);
    }
    free(list);
}

int insert_user(users_node* list, struct_user user){
    users_node* new_node = (users_node*) malloc(sizeof(users_node));
    if(new_node == NULL){
        return 0;
    }

    new_node->user = user;
    new_node->next = NULL;

    if(is_users_list_empty(list)){
        list -> next = new_node;
    } else{
        users_node* aux = list -> next;
        while(aux->next != NULL){
            aux = aux->next;
        }
        aux->next = new_node;
    }
    return 1;
}

void print_users_list(users_node* list){
    users_node* aux = list->next;
    while(aux != NULL){
        printf("--------------------\n");
        printf("Username: %s\n", aux->user.username);
        printf("Password: %s\n", aux->user.password);
        printf("Type: %s\n", aux->user.type);
        printf("--------------------\n");
        aux = aux->next;
    }
    free(aux);
    return;
}

users_node* search_user_from_list(users_node* list, char* username){
    users_node* aux = list;
    while(aux != NULL){
        if(strcmp(aux->user.username, username) == 0){
            return aux;
        }
        aux = aux->next;
    }
    free(aux);
    return NULL;
}

int remove_user_from_list(users_node* list, char* username){
    users_node* aux = list;
    users_node* prev = NULL;
    users_node* node_to_remove = search_user_from_list(list, username);
    
    if(node_to_remove == NULL){
        return 0;
    }
    
    while(aux != NULL){
        if(aux == node_to_remove){
            if(prev == NULL){
                list = aux->next;
            } else{
                prev->next = aux->next;
            }
            free(aux);
            return 1;
        }
        prev = aux;
        aux = aux->next;
    }
    free(aux);
    free(prev);
    free(node_to_remove);
    return 0;
}