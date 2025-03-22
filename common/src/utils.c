/*
 * This file implements the utility module, holding functions reusable for multiple assignments.
 */

#include "utils.h"
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>

void sleep_for_ms(long long delay_in_ms)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delay_ns = delay_in_ms * NS_PER_MS;
    int seconds = delay_ns / NS_PER_SECOND;
    int nanoseconds = delay_ns % NS_PER_SECOND;
    struct timespec req_delay = {seconds, nanoseconds};
    nanosleep(&req_delay, (struct timespec *) NULL);
}

bool has_timeout_passed(time_t start_time, int timeout_seconds) 
{
    time_t current_time = time(NULL);
    return difftime(current_time, start_time) >= timeout_seconds;
}

long long get_time_in_ms(void) 
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nano_seconds = spec.tv_nsec;
    long long milli_seconds = seconds * 1000 + nano_seconds / 1000000;
    return milli_seconds;
}

long long get_time_in_us(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

long long get_time_in_ns(void) 
{
    struct timespec spec;
    clock_gettime(CLOCK_BOOTTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec + seconds * 1000*1000*1000;
	assert(nanoSeconds > 0);
    
    static long long lastTimeHack = 0;
    assert(nanoSeconds > lastTimeHack);
    lastTimeHack = nanoSeconds;

    return nanoSeconds;
}

void trim_newline(char *str) 
{
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
}


