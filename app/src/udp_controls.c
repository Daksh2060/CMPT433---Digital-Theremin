#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


#define UDP_PORT 12345
#define MAX_BUFFER_SIZE 1600 

static int socket_descriptor;
static pthread_t udp_thread;

static struct sockaddr_in client_addr;
static socklen_t addr_len = sizeof(client_addr);

static volatile bool running = true;
volatile bool end_program = false;

static void *udp_listener(void *arg);
static void process_string(const char *cmd);

void trim_newline(char *str) 
{
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
}

void udp_init(void) 
{
    if (pthread_create(&udp_thread, NULL, udp_listener, NULL) != 0) {
        perror("Failed to create UDP listener thread");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }
}

static void send_response(const char *msg) 
{
    sendto(socket_descriptor, msg, strlen(msg), 0, (struct sockaddr *)&client_addr, addr_len);
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
    printf("Received string: %s\n", cmd);
    
    if (strcasecmp(cmd, "stop") == 0) {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Terminating Program.\n");
        send_response(buffer);
        end_program = true;
    } 
}

void udp_cleanup(void) 
{
    running = false;
    shutdown(socket_descriptor, SHUT_RDWR);
    pthread_join(udp_thread, NULL);
    close(socket_descriptor);
}

int main() {
    udp_init();
    while (!end_program) {
        sleep(1);
    }
    udp_cleanup();
    return 0;
}