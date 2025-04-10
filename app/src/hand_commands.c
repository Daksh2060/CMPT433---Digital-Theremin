/*
 * This file implements the hand commands module, handling the processing of hand commands
 * and translating them into corresponding actions for the sine mixer. It takes input from
 * the MediaPipe hand tracking system and translates it into alterations to the currently
 * playing wave.
 */

#include "hand_commands.h"
#include "sine_mixer.h"
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define NUM_COMMANDS 14  // Number of commands in the array
#define NOTE_A4_BASE 440 // Base frequency for A4 note

// Binary representations of the hand commands
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

// Current octave level based on open hand
static int currentOctave = 0;

// Track the last played note to avoid reprocessing
static double last_note = 0.0;

// Track previous command to avoid reprocessing
static volatile int command = -1;

// Thread control variables
static pthread_mutex_t lock;
static pthread_t thread;
static bool end_thread = false;

// Structure to map binary command to note frequency
struct digit_to_play_freq{
    int binary;
    int note_offset;
};

// Commands mapping each hand gesture to a note
static struct digit_to_play_freq commands[] = {
    {B0000, -9}, // C
    {B0001, -8}, // C sharp
    {B1000, -7}, // D
    {B1001, -6}, // D sharp
    {B0100, -5}, // E
    {B0101, -4}, // E sharp (F)
    {B0010, -4}, // F
    {B0011, -3}, // F sharp
    {B1100, -2}, // G
    {B1101, -1}, // G sharp
    {B0110, 0},  // A
    {B0111, 1},  // A sharp
    {B1110, 2},  // B
    {B1111, 3}   // B sharp (C)
};

// Helper function prototypes
static void process_command(int cmd);
static void *command_thread(void *arg);
static double note_to_freq(int offset);
static void play_note(double freq);

void command_handler_init()
{
    pthread_mutex_init(&lock, NULL);
    if (pthread_create(&thread, NULL, command_thread, NULL) != 0){
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

int command_handler_getOctave()
{
    return currentOctave;
}

void command_handler_setOctave(int octave)
{
    currentOctave = octave;
}

void command_handler_cleanup()
{
    end_thread = true;
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&lock);
}

// Function to process the command and play the corresponding note
static void process_command(int cmd)
{
    for (int i = 0; i < NUM_COMMANDS; i++){
        if (cmd == commands[i].binary){
            double frequency = note_to_freq((12 * currentOctave) + commands[i].note_offset);
            play_note(frequency);
        }
    }
}

// Thread function that continuously processes commands
static void *command_thread(void *arg)
{
    (void)arg;
    while (!end_thread){
        if (command != -1){
            pthread_mutex_lock(&lock);
            {
                process_command(command);
            }
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

// Function to convert note offset to frequency
static double note_to_freq(int offset)
{
    return pow(2, (double)offset / 12) * NOTE_A4_BASE;
}

// Function to play the note at the given frequency
static void play_note(double frequency)
{
    if (frequency != last_note){
        sine_mixer_queue_frequency(frequency);
        last_note = frequency;
    }
}