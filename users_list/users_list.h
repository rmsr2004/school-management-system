#ifndef USERS_LIST__H
#define USERS_LIST__H

#include "../user/user.h"

/*
* Users list node. 
* Contains: 
* - data: user data.
* - next: pointer to the next node.
*/
typedef struct users_node{
    struct_user user;
    struct users_node* next;
} users_node;

/*
* Creates a new users list node.
*/
users_node* create_users_list();
/*
* Verify if users list is empty.
* @return 1 if the list is empty, 0 otherwise.
*/
int is_users_list_empty(users_node* list);
/*
* Destroy a users list.
*/
void destroy_users_list(users_node* list);
/*
* Insert a new user in the users list.
* @param List users list.
* @param User user to be inserted.
* @return 1 if the user was inserted, 0 otherwise.
*/
int insert_user(users_node* list, struct_user user);
/*
* Print users list.
*/
void print_users_list(users_node* list);
/*
* Search for a user in the users list.
* @param username Username to be searched.
*/
users_node* search_user_from_list(users_node* list, char* username);
/*
* Remove a user from the users list.
* @param username Username to be removed.
*/
int remove_user_from_list(users_node* list, char* username);

#endif