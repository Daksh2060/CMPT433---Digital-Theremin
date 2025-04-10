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
#define B1001 9
#define B0101 5
#define B1101 13

static int currentOctave = 0;

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

#define NUM_COMMANDS 14

// index, middle, and ring control base notes (white keys)
// pinky controls sharps (black keys)

static struct digit_to_play_freq commands[] = {
    {B0000,-9}, // C
    {B0001,-8}, // C sharp
    {B1000,-7}, // D
    {B1001,-6}, // D sharp 
    {B0100,-5}, // E
    {B0101,-4}, // E sharp (F)
    {B0010,-4}, // F
    {B0011,-3}, // F sharp
    {B1100,-2}, // G
    {B1101,-1}, // G sharp 
    {B0110,0}, // A 
    {B0111,1}, // A sharp
    {B1110,2}, // B
    {B1111,3} // B sharp (C)
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

int command_handler_getOctave()
{
    return currentOctave;
}

void command_handler_setOctave(int octave)
{
    currentOctave = octave;
}

static void process_command(int cmd) 
{
    // find the member in the struct array that corresponds to cmd
    for(int i = 0; i < NUM_COMMANDS; i++) {
        if(cmd == commands[i].binary) {
            double frequency = note_to_freq((12*currentOctave)+commands[i].note_offset);
            // printf("freq:%f\n",frequency);
            
            play_note(frequency);
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
        //printf("frequency: %f\n",frequency);
        SineMixer_queueFrequency(frequency);
        last_note = frequency;
    }
}