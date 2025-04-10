#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "hand_commands.h"
#include "utils.h"
#include "lcd_menus.h"

#define UDP_PORT 12345
#define MAX_BUFFER_SIZE 1600 

#define NUM_HANDLERS 6
#define CMD_SIZE 128

char prev_cmd[CMD_SIZE] = "";

static int socket_descriptor;
static pthread_t udp_thread;

static struct sockaddr_in client_addr;
static socklen_t addr_len = sizeof(client_addr);

static volatile bool running = true;

static void *udp_listener(void *arg);
static void process_string(const char *cmd);

static int binary_string_to_integer(const char* str);

void udp_init(void) 
{
    if (pthread_create(&udp_thread, NULL, udp_listener, NULL) != 0) {
        perror("Failed to create UDP listener thread");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }
}

static void *udp_listener(void *arg) 
{
    (void)arg;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(UDP_PORT);

    socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0) {
        perror("Failed to create UDP socket");
        exit(EXIT_FAILURE);
    }

    if (bind(socket_descriptor, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Failed to bind UDP socket");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_BUFFER_SIZE]; 

    while (running) {
        ssize_t n = recvfrom(socket_descriptor, buffer, MAX_BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            trim_newline(buffer);
            process_string(buffer);
        }
    }
    return NULL;
}

static void process_string(const char *cmd) 
{
    //copy the command, safer for strtok
    char cmd_copy[strlen(cmd) + 1];
    strcpy(cmd_copy, cmd);
    //extract first token
    int num_tokens = 43; 
    int token_index = 0;  
    char *tokens[num_tokens];
    tokens[token_index] = strtok(cmd_copy, " ");
     //extract the rest of tokens and copy to integer buffer
    int data_points[num_tokens - 1];
    while(tokens[token_index] != NULL && token_index < num_tokens){
      token_index++;
      tokens[token_index] = strtok(NULL, " ");
    }
    for(int i = 1; i < num_tokens; i++){
      data_points[i - 1] = atoi(tokens[i]);
    }
    set_keypoint_buff(data_points, num_tokens - 1);

    //if its a new command, we parse
    if (strcasecmp(tokens[0], prev_cmd) != 0) {
      strncpy(prev_cmd, tokens[0], CMD_SIZE - 1);
      prev_cmd[CMD_SIZE - 1] = '\0';

      //char *endptr;
      // previously, was just taking the raw 1100 and converting into int
      // meaning 1100 meant 1100 and not 14
      int value = binary_string_to_integer(tokens[0]); //strtol(cmd, &endptr, 2);
      // TODO: sanecheck value?
      command_handler_update_current_command(value);
    }
}

void udp_cleanup(void) 
{

    running = false;
    shutdown(socket_descriptor, SHUT_RDWR);
    pthread_join(udp_thread, NULL);
    close(socket_descriptor);
}

static int binary_string_to_integer(const char* str) 
{
    int out = 0;
    int len = 3;
    //printf("len: %d\n", len);
    for(int i = 0; str[i] != '\0'; i++) {
        out = out | ((str[i] - '0') << (len - i));
    }
    //printf("binary conversion: in:%s, out:%d\n",str,atoi(str));
    return atoi(str);
}
