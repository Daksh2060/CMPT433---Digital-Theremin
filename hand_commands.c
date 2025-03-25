#include "hand_commands.h"
#include <stdio.h>
#include <string.h>
#include "sine_mixer.h"

void process_0000(){

    printf("OPEN_HAND\n");
}

void process_1000(){

    printf("THUMB_INDEX\n");
}

void process_0100(){

    printf("THUMB_MIDDLE\n");
}

void process_0010(){
    
    printf("THUMB_RING\n");
}

void process_0001(){
    
    printf("THUMB_PINKY\n");
}

void process_0101(){
    
    printf("INDEX_MIDDLE_THUMB\n");
}