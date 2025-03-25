#include "hand_commands.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static volatile int command = -1;       
static pthread_mutex_t lock;            
static pthread_t thread;             

static void process_command(int cmd);
static void* command_thread(void* arg); 


void command_handler_init() 
{
    pthread_mutex_init(&lock, NULL);

    if (pthread_create(&thread, NULL, command_thread, NULL) != 0) {
        perror("Failed to create command thread");
    }
}


void update_current_command(int cmd) 
{
    pthread_mutex_lock(&lock);
    {
        command = cmd; 
    }                       
    pthread_mutex_unlock(&lock);
}


void command_handler_cleanup() 
{
    pthread_cancel(thread);     
    pthread_join(thread, NULL);       
    pthread_mutex_destroy(&lock);      
}


static void process_command(int cmd) 
{
    printf("Executing command: %d\n", cmd);
    command = -1;
}


static void* command_thread(void* arg) 
{
    while (1) {
        if (command != -1) {
            pthread_mutex_lock(&lock);
            {
                process_command(command);
            }  
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}
