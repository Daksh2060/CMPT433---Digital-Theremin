#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

//initialize state machine, creates thread and passes to rotary_statemachine_doState()
//thread hidden from direct access
void rotary_encoder_init();
//cleans up thread
void rotary_encoder_cleanup();



#endif