#include "telemetry.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Shared Memory Mapping (Anonymous, Shared between processes)
    telemetry_monitor* tm = mmap(NULL, sizeof(telemetry_monitor), 
        PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the monitor structure in shared memory
    init_monitor(tm);

    // Fork the Transmitter Process
    pid_t pid_tx = fork(); 
    if (pid_tx == 0) {
        process_transmit_manager(tm);
        exit(0);
    }

    // Fork the Sensor Acquisition Process
    pid_t pid_sens = fork(); 
    if (pid_sens == 0) {
        process_sensors_manager(tm);
        exit(0);
    }

    // Parent Process: Let the simulation run for a set time
    printf("[MAIN] Simulation started for 5 seconds...\n");
    sleep(5); 

    printf("[MAIN | PID: %d] Time is up. Killing child processes...\n", getpid());
    
    // Kill both children to stop the simulation
    kill(pid_sens, SIGKILL);
    kill(pid_tx, SIGKILL);
    
    // Clean up zombie processes
    wait(NULL);
    wait(NULL);
    
    printf("[MAIN | PID: %d] Session done. Cleaning memory.\n", getpid());
    
    // Clean dynamic memory and unmap shared memory
    pthread_mutex_destroy(&tm->telemetry_mutex);
    pthread_cond_destroy(&tm->not_empty);
    pthread_cond_destroy(&tm->not_full);
    munmap(tm, sizeof(telemetry_monitor));
    
    return 0;
}