/*
 * This file manages event timestamp collection, calculates time statistics
 * for events, and provides functions to record events and retrieve statistics.
 * The module supports multi-threading and ensures thread safety using mutexes.
 * Written by Brian Fraser (with modifications)
 */

#include "period_timer.h"
#include "utils.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MAX_EVENT_TIMESTAMPS (1024*8)   // The maximum capacity of the timestamp buffer
#define MS_PER_NS (1000*1000.0)         // Conversion factor from nanoseconds to milliseconds

// Store the timestamp samples each time we mark an event.
typedef struct {
    long timestamp_count;                               // Tracks the number of timestamps recorded for an event
    long long timestamps_in_ns[MAX_EVENT_TIMESTAMPS];   // Array to store event timestamps in nanoseconds
    long long prev_timestamp_in_ns;                     // Stores the previous timestamp for calculating time deltas between events
} TimeStamps;

// Array to store timestamp data for different period events
static TimeStamps event_data[NUM_PERIOD_EVENTS];

// Mutex for thread-safety when modifying event data
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

// Flag to track whether the period timer system has been initialized
static bool is_initialized = false;

// Function Prototypes to update statistics for a particular event
static void updateStats(TimeStamps *data, PeriodStatistics *stats);

void period_init(void)
{
    memset(event_data, 0, sizeof(event_data[0]) * NUM_PERIOD_EVENTS);
    is_initialized = true;
}

void period_mark_event(enum PeriodWhichEvent event)
{
    assert (event >= 0 && event < NUM_PERIOD_EVENTS); 
    assert (is_initialized);

    TimeStamps *data = &event_data[event];
    pthread_mutex_lock(&s_lock);
    {
        if (data->timestamp_count < MAX_EVENT_TIMESTAMPS) {
            data->timestamps_in_ns[data->timestamp_count] = get_time_in_ns();
            data->timestamp_count++;
        } 
        else {
            printf("WARNING: No sample space for event collection on %d\n", event);
        }
    }
    pthread_mutex_unlock(&s_lock);
}

void period_get_statistics_and_clear(enum PeriodWhichEvent event, PeriodStatistics *stats)
{
    assert (event >= 0 && event < NUM_PERIOD_EVENTS);
    assert (is_initialized);

    TimeStamps *data = &event_data[event];
    pthread_mutex_lock(&s_lock);
    {
        updateStats(data, stats);

        if (data->timestamp_count > 0){
            data->prev_timestamp_in_ns = data->timestamps_in_ns[data->timestamp_count - 1];
        }
        data->timestamp_count = 0;
    }
    pthread_mutex_unlock(&s_lock);
}

void period_cleanup(void)
{
    is_initialized = false;
}

// Helper function to calculate statistics (min, max, avg) for event timestamps
static void updateStats(TimeStamps *data, PeriodStatistics *stats)
{
    long long prev_in_ns = data->prev_timestamp_in_ns;

    // Handle startup (no previous sample)
    if (prev_in_ns == 0){
        prev_in_ns = data->timestamps_in_ns[0];
    }
    
    // Find min/max/sum time delta between consecutive samples
    long long sum_deltas_ns = 0;
    long long min_ns = 0;
    long long max_ns = 0;
    for (int i = 0; i < data->timestamp_count; i++){
        long long this_time = data->timestamps_in_ns[i];
        long long delta_ns = this_time - prev_in_ns;

        sum_deltas_ns += delta_ns;

        if (i == 0 || delta_ns < min_ns) {
            min_ns = delta_ns;
        }
        if (i == 0 || delta_ns > max_ns) {
            max_ns = delta_ns;
        }
        prev_in_ns = this_time;
    }

    long long avg_ns = 0;
    if (data->timestamp_count > 0) {
        avg_ns = sum_deltas_ns / data->timestamp_count;
    } 

    // Convert to milliseconds and store in stats
    stats->min_period_ms = min_ns / MS_PER_NS;
    stats->max_period_ms = max_ns / MS_PER_NS;
    stats->avg_period_ms = avg_ns / MS_PER_NS;
    stats->num_samples = data->timestamp_count;
}



