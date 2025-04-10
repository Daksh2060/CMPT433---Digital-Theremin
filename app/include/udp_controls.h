/*
 * This module is used to interface with the UDP controls.
 * It allows the user to send and receive UDP packets for communication.
 */

#ifndef _UDP_CONTROLS_H_
#define _UDP_CONTROLS_H_

#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define UDP_PORT 12345        // Port used for UDP communication
#define MAX_BUFFER_SIZE 1600  // Maximum size of the UDP buffer


/*
 * Initializes the UDP communication and starts the thread.
 */
void udp_init();


/*
 * Cleans up the UDP communication.
 */
void udp_cleanup(void);

#endif