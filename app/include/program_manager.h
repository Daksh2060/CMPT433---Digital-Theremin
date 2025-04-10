/*
 * This module is used to manage the program's lifecycle.
 * It handles the initialization, cleanup, and waiting for the program to end.
 */

#ifndef _PROGRAM_MANAGER_H_
#define _PROGRAM_MANAGER_H_


/*
 * This function initializes the program manager.
 */
void program_manager_init();


/*
 * This function waits for the program to end via a flag.
 */
void program_wait_to_end();


/* 
 * This function cleans up the program manager.
 */
void program_manager_cleanup();

#endif