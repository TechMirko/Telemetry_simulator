#include "telemetry.h"

void init_monitor(telemetry_monitor* tm) {
    tm->head = 0;
    tm->tail = 0;
    tm->counter = 0;

    // Initialize Mutex with Process Shared attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&tm->telemetry_mutex, &attr);

    // Initialize Condition Variables with Process Shared attribute
    pthread_condattr_t cond;
    pthread_condattr_init(&cond);
    pthread_condattr_setpshared(&cond, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&tm->not_full, &cond);
    pthread_cond_init(&tm->not_empty, &cond);

    // Cleanup attribute objects
    pthread_mutexattr_destroy(&attr);
    pthread_condattr_destroy(&cond);
}

void write_packet(telemetry_monitor* tm, sensors_type sens, float val) {
    pthread_mutex_lock(&tm->telemetry_mutex); 

    // Wait if buffer is full
    while (tm->counter == BUFFER_SIZE)
        pthread_cond_wait(&tm->not_full, &tm->telemetry_mutex);
    
    // Write data to the buffer at the tail index
    tm->packets[tm->tail].sensor = sens;
    tm->packets[tm->tail].value = val;
    tm->packets[tm->tail].time_stamp = time(NULL);

    // Circular increment of tail
    tm->tail = (tm->tail + 1) % BUFFER_SIZE;
    tm->counter++;

    // Signal that the buffer is not empty anymore
    pthread_cond_signal(&tm->not_empty);
    pthread_mutex_unlock(&tm->telemetry_mutex); 
}

telemetry_packet read_packet(telemetry_monitor* tm) {
    pthread_mutex_lock(&tm->telemetry_mutex);

    // Wait if buffer is empty
    while (tm->counter == 0)
        pthread_cond_wait(&tm->not_empty, &tm->telemetry_mutex);

    // Read data from the head index
    telemetry_packet pkt = tm->packets[tm->head];
    
    // Circular increment of head
    tm->head = (tm->head + 1) % BUFFER_SIZE;
    tm->counter--;

    // Signal that there is space in the buffer
    pthread_cond_signal(&tm->not_full); 
    pthread_mutex_unlock(&tm->telemetry_mutex); 

    return pkt;
}

void* thread_rpm(void* arg) {
    telemetry_monitor* tm = (telemetry_monitor*)arg;
    while (1) {
        float rpm = 1000.0 + (rand() % 9000);
        write_packet(tm, SENSOR_RPM, rpm);
        usleep(100000); // 100ms (10Hz)
    }
    return NULL;
}

void* thread_temp(void* arg) {
    telemetry_monitor* tm = (telemetry_monitor*)arg;
    while (1) {
        float temp = 80.0 + (rand() % 40);
        write_packet(tm, SENSOR_TEMP, temp);
        usleep(500000); // 500ms (2Hz)
    }
    return NULL;
}

void process_sensors_manager(telemetry_monitor* tm) {
    printf("[MGR] Starting Sensors Process (PID %d)\n", getpid());
    pthread_t rpm, temp;
    pthread_create(&rpm, NULL, thread_rpm, tm);
    pthread_create(&temp, NULL, thread_temp, tm);
    
    // Wait for threads (infinite loop in this simulation)
    pthread_join(rpm, NULL);
    pthread_join(temp, NULL);
}

void process_transmit_manager(telemetry_monitor* tm) {
    printf("[MGR] Starting Transmitter Process (PID %d)\n", getpid());

    mosquitto_lib_init(); // Initialize MQTT library

    // Create MQTT instance
    struct mosquitto *mosq = mosquitto_new("Telemetry simulator", true, NULL);
    if (!mosq) {
        perror("Error creating Mosquitto instance.\n");
        exit(1);
    }

    // Connect to broker (localhost)
    if (mosquitto_connect(mosq, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect to the broker.\n");
        exit(1);
    }

    // Start the network loop in a background thread
    mosquitto_loop_start(mosq);
    
    while (1) {
        telemetry_packet pkt = read_packet(tm);

        char topic[50];
        char payload[100];

        // Prepare the Topic
        char* name = (pkt.sensor == SENSOR_RPM) ? "RPM" : "TEMP";
        sprintf(topic, "fsae/car/%s", name);
        sprintf(payload, "{\"val\": %.2f, \"ts\": %ld}", pkt.value, pkt.time_stamp);

        // Publish the message
        int rc = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
        
        if (rc != MOSQ_ERR_SUCCESS) {
            printf("[MQTT ERROR] Publish error: %s\n", mosquitto_strerror(rc));
        } else {
            printf("    -->[MQTT] Sent to %s: %s\n", topic, payload);
        }
    }

    // Cleanup (unreachable code in this while(1) example, but good practice)
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}