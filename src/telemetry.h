#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <mosquitto.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h> 

#define BUFFER_SIZE 20

// ----- DATA STRUCTURES ----- //

// Enumeration for sensor types
typedef enum {SENSOR_RPM, SENSOR_TEMP} sensors_type;

// Telemetry packet containing the sensor data
typedef struct {
    sensors_type sensor;
    float value;
    long time_stamp;
} telemetry_packet;

// Monitor structure to handle shared memory synchronization
typedef struct {
    telemetry_packet packets[BUFFER_SIZE]; // Circular buffer
    int head;       // Read index
    int tail;       // Write index
    int counter;    // Number of elements in buffer
    pthread_mutex_t telemetry_mutex; // Mutex for mutual exclusion
    pthread_cond_t not_full;         // CV: wait here if buffer is full
    pthread_cond_t not_empty;        // CV: wait here if buffer is empty
} telemetry_monitor;

// ----- FUNCTIONS PROTOTYPES ----- //

/**
 * Initializes the monitor, including the mutex and condition variables
 * with PTHREAD_PROCESS_SHARED attributes to work across processes.
 * @param tm Pointer to the telemetry monitor
 */
void init_monitor(telemetry_monitor* tm);

/**
 * Produces a telemetry packet and writes it into the monitor buffer.
 * Thread-safe and process-safe.
 * @param tm Pointer to the telemetry monitor
 * @param sens Type of the sensor (RPM or TEMP)
 * @param val Value read by the sensor
 */
void write_packet(telemetry_monitor* tm, sensors_type sens, float val);

/**
 * Consumes a packet from the buffer. Blocks if the buffer is empty.
 * @param tm Pointer to the telemetry monitor
 * @return The telemetry packet read from the buffer
 */
telemetry_packet read_packet(telemetry_monitor* tm);

/**
 * Thread routine: Simulates an RPM sensor.
 * Generates random values and writes to the monitor at a specific frequency.
 * @param arg Pointer to the telemetry_monitor
 */
void* thread_rpm(void* arg);

/**
 * Thread routine: Simulates a Temperature sensor.
 * Generates random values and writes to the monitor at a specific frequency.
 * @param arg Pointer to the telemetry_monitor
 */
void* thread_temp(void* arg);

/**
 * Manages the "Acquisition" process.
 * Spawns the sensor threads and waits for them.
 * @param tm Pointer to the telemetry monitor
 */
void process_sensors_manager(telemetry_monitor* tm);

/**
 * Manages the "Transmission" process.
 * Initializes the Mosquitto MQTT client, reads from the monitor,
 * and publishes data to the broker.
 * @param tm Pointer to the telemetry monitor
 */
void process_transmit_manager(telemetry_monitor* tm);

#endif