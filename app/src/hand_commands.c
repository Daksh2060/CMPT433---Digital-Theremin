#include "hand_commands.h"
#include "sine_mixer.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h> // for pow
#include <stdbool.h>

// some very bad code to avoid having to manually enable binary
// literals in the compiler
#define B0000 0
#define B1000 8
#define B0100 4
#define B0010 2
#define B0001 1
#define B1100 12
#define B0110 6
#define B0011 3
#define B1110 14
#define B0111 7 
#define B1111 15

static volatile int command = -1;       
static pthread_mutex_t lock;            
static pthread_t thread;    

static bool end_thread = false;

static void process_command(int cmd);
static void* command_thread(void* arg); 

struct digit_to_play_freq {
    int binary;
    int note_offset;
}; 

#define NUM_COMMANDS 11

static struct digit_to_play_freq commands[] = {
    // single finger notes
    {B0000,0}, // this plays a note given an open hand 
    {B1000,2},
    {B0100,3},
    {B0010,5},
    {B0001,7},
    // double finger notes
    {B1100,8},
    {B0110,10},
    {B0011,12},
    // triple finger notes
    {B1110,14},
    {B0111,15},
    // all fingers note
    {B1111,17}
};

// function that returns the frequency of a note
// given its semitone offset from A4
static double note_to_freq(int offset);

// function to play a note given its frequency
// abstraction for if the sinemixer in HAL changes.
static void play_note(double freq);

void command_handler_init() 
{
    pthread_mutex_init(&lock, NULL);

    if (pthread_create(&thread, NULL, command_thread, NULL) != 0) {
        perror("Failed to create command thread");
    }
}

void command_handler_update_current_command(int cmd) 
{
    pthread_mutex_lock(&lock);
    {
        command = cmd; 
    }                       
    pthread_mutex_unlock(&lock);
}


void command_handler_cleanup() 
{
    end_thread = true;    
    pthread_join(thread, NULL);       
    pthread_mutex_destroy(&lock);      
}

static void process_command(int cmd) 
{

    // find the member in the struct array that corresponds to cmd
    for(int i = 0; i < NUM_COMMANDS; i++) {
        if(cmd == commands[i].binary) {
            play_note(note_to_freq(commands[i].note_offset));
        }
    }
}


static void* command_thread(void* arg) 
{
    (void)arg;
    while (!end_thread) {
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

#define NOTE_A4_BASE 440

static double note_to_freq(int offset)
{
    return pow(2,(double)offset/12) * NOTE_A4_BASE;
}

static double last_note = 0.0;

static void play_note(double frequency)
{
    if(frequency != last_note) {
        printf("frequency: %f\n",frequency);
        SineMixer_queueFrequency(frequency);
        last_note = frequency;
    }
}