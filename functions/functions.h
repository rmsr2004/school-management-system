#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "../user/user.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

/*-----------------------------------------------------------------------------------------------------*/
#define BUFFER_LEN      200         // Buffer length.
#define INPUT_SIZE      100         // Input size.
#define ARGS_LEN        20          // Arguments length (.i.e username, password, etc.)         
#define MAX_FILE_LEN    20          // Max file length.
#define MSQ_QUEUE_PATH  "/tmp"      // Message queue path.
#define MAX_USERS       30          // Max users.
#define CLASSES_SIZE    30          // Max classes.
/*-----------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------*
*                                       GLOBAL VARIABLES                                               *
*------------------------------------------------------------------------------------------------------*/
extern char* config_file;                               // Config file.
extern int shmid;                                       // Shared memory id.
extern int client_id;                                   // Client ID.
/*---------------------------------------------SERVER--------------------------------------------------*/
extern pid_t tcp_pids[MAX_USERS];                       // Array to save all processes id that are 
                                                        // responsible to handle TCP connections
extern int tcp_pids_index;                              // Current TCP connections.
extern pid_t tcp_pid;                                   // TCP connection manager pid.
extern pid_t udp_pid;                                   // UDP connection pid.
extern struct sockaddr_in server_udp;                   // UDP socket address.
extern struct sockaddr_in server_tcp;                   // TCP socket address.
extern struct sockaddr_in client_tcp;                   // TCP client address.
extern int client_addr_size;                            // TCP client address size.
extern int tcp_socket;                                  // TCP socket.
extern int udp_socket;                                  // UDP socket.
/*---------------------------------------------UDP_ADMIN------------------------------------------------*/
extern int recv_len;                                    // Received length
extern char* input;                                     // Input message
extern char* buffer;                                    // Buffer message
extern struct sockaddr_in server;                       // Server address
extern socklen_t slen;                                  // Server length
/*---------------------------------------------TCP_CLIENT------------------------------------------------*/
extern int fd;                                          // TCP socket file descriptor.
extern int multicast_sock[CLASSES_SIZE];                // Multicast socket file descriptor.
extern int multicast_index;                             // Multicast Process Index.
extern struct ip_mreq multicast_mreq[CLASSES_SIZE];     // Multicast Request.
extern int logged_in;                                   // Flag to check if the user is logged in.
extern int last_port_used;                              // Last port used to join the multicast group.
extern char* buffer;                                    // Buffer to store the messages.
extern char* input;                                     // Buffer to store the input.
extern pid_t pid;                                       // Read Message Queue Process ID.
extern pid_t multicast_pid[CLASSES_SIZE];               // Multicast Process ID.
extern pid_t father_pid;                                // Father Process ID.
/*-----------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------*
*                                            FUNCTIONS                                                 *
*------------------------------------------------------------------------------------------------------*/

/*
*   Defines config_file variable to use globaly.
*   @param filename File's filename.
*/
void define_config_file(char* filename);
/*
*   Verify login.
*   @param user Struct containing user login information
*   @param connection_type "UDP" or "TCP"
*   @return 1 if success, 0 otherwise
*/
int verify_login(struct_user* user, char* connection_type);
/*
*   Get time in format "HH:MM:SS".
*/
char* get_time();
/*
*   Returns user from file if username exists.
*/
struct_user* get_user_from_file(char* username);
/*
*   Creates an array with arguments from user.
*/
char** create_args_array(int args_number);
/*
*   Verify if string has numbers.
*/
int is_number(char* string);
/*
*   Convert string to caps.
*/
char* to_upper(char* string);
/*
*   Verify if string hasn´t numbers.
*/
int verify_string(char* string);
/*
*   Invoke an error on system.
*   @param Message to describe the error.
*/
void error(const char *format, ...);
/* 
*   Removes '\\n' from string.
*/
void remove_line_break(char* string);

#endif // FUNCTIONS_H