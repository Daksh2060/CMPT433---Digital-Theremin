/*
 * This module records and reports the timing of periodic events.
 * It allows the application to track the timing of events (e.g., sampling a sensor) 
 * by recording timestamps and calculating statistics such as minimum, maximum, and average periods.
 * By: Brian Fraser (with modifications)
 * 
 * Usage:
 *  1. Call `period_markEvent()` periodically to mark each occurrence of the event, such as
 *     each time you sample the A2D.
 *  2. Call `period_get_statistics_and_clear()` to retrieve the statistics for an event of interest.
 */

#ifndef _PERIOD_TIMER_H_
#define _PERIOD_TIMER_H_

// Enum defining different periodic events of interest.
enum PeriodWhichEvent {
    PERIOD_EVENT_SAMPLE_ACCELEROMETER,
    PERIOD_EVENT_SAMPLE_AUDIO,
    NUM_PERIOD_EVENTS          // Ensure this is last
};

// Struct to store statistics about a periodic event's timing in milliseconds.
typedef struct {
    int num_samples;          // The number of samples recorded for the event
    double min_period_ms;     // The minimum period between event occurrences
    double max_period_ms;     // The maximum period between event occurrences
    double avg_period_ms;     // The average period between event occurrences
} PeriodStatistics;


/**
 * Initializes the module's data structures.
 */
void period_init(void);


/**
 * Records the current time as a timestamp for the indicated event.
 * This allows later calls to `period_get_statistics_and_clear()` to access 
 * these timestamps and compute the timing statistics for the periodic event.
 * 
 * @param event The event for which to mark the timestamp.
 */
void period_mark_event(enum PeriodWhichEvent event);


/**
 * Fills the `pStats` struct with the statistics about the periodic event.
 * This function is thread-safe and may be called by any thread.
 * After computing the statistics, it clears the data stored for the event.
 * 
 * @param event The event for which to get statistics.
 * @param stats A pointer to a `PeriodStatistics` struct to fill with the statistics.
 */
void period_get_statistics_and_clear(enum PeriodWhichEvent event, PeriodStatistics *stats);


/**
 * Cleans up the module's data structures.
 */
void period_cleanup(void);

#endif
 