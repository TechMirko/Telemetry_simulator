# Formula SAE Telemetry Simulator 🏎️💨
A high-performance telemetry simulation system designed for Formula SAE vehicles. This project demonstrates advanced IPC (Inter-Process Communication) techniques using Shared Memory Monitors, Multithreading, and MQTT for real-time data streaming.

## 📋 Project Overview
The system simulates a vehicle's data acquisition unit. It generates sensor data (RPM and Temperature) in real-time, safely stores it in a circular buffer located in shared memory, and transmits it via MQTT to a remote listener.

### Key Architecture Features
**Hybrid Concurrency:** Combines Multiprocessing (Process isolation for fault tolerance) and Multithreading (Lightweight sensor acquisition).

**Shared Memory Monitor:** Uses mmap with MAP_SHARED to create a memory region accessible by multiple processes.

**Synchronization:** Implements a thread-safe and process-safe Monitor Pattern using Mutexes and Condition Variables configured with PTHREAD_PROCESS_SHARED.

**IoT Connectivity:** Uses Eclipse Mosquitto (libmosquitto) to publish JSON-formatted telemetry packets.

**Visualization:** Includes a Python script to subscribe and display data streams in real-time.

## 🛠️ Prerequisites
**System Dependencies**
- **Linux Environment** (Required for ```mmap```, ```fork``` and ```signals```).

- **GCC Compiler with POSIX thread support (```-pthread```)**.

- **Mosquitto Broker and Development Library:**

    - Arch Linux: ```sudo pacman -S mosquitto```

    - Ubuntu/Debian: ```sudo apt install mosquitto libmosquitto-dev```

**Python Dependencies**
- Python 3.x

- Paho MQTT library (Version 2.0+ recommended due to CallbackAPIVersion.VERSION2 usage): ```pip install paho-mqtt```

## 📂 File Structure
```main.c```: Entry point. Sets up shared memory, handles process forking (Sensors & Transmitter), and manages the simulation lifecycle (5-second timer).

```telemetry.c```: Implementation of the Monitor pattern, sensor threads (Producer), and the MQTT transmitter (Consumer).

```telemetry.h```: Header file defining the ```telemetry_monitor``` structure, packet types, and function prototypes.

```plotter.py```: Python client that subscribes to fsae/car/# and prints received telemetry.

## 🚀 Compilation & Running
1. **Start the MQTT Broker:** ensure the Mosquitto broker is running in the background: ```sudo systemctl start mosquitto``` or simply run: ```mosquitto -v```

2. **Compile the C System:** you must link against pthread (threading) and mosquitto (MQTT) ```gcc -pthread main.c telemetry.c -o telemetry -lmosquitto``` by ```make``` command

3. **Run the Python Listener:** open a terminal to act as the ground station ```python plotter.py``` it will display ```[PYTHON] Waiting connection....```

4. **Run the Telemetry Simulator** open a separate terminal and run the compiled executable ```./telemetry```

## 🧠 Technical Details
**The Monitor Pattern**

- The core synchronization mechanism is defined in telemetry.h. It uses a circular buffer (```packets[BUFFER_SIZE]```) protected by:
    - ```telemetry_mutex```: Ensures mutual exclusion during writing/reading.

    - ```not_full```: Condition variable where producers wait if the buffer is full.

    - ```not_empty```: Condition variable where the consumer waits if the buffer is empty.

**Data Flow**

- **Sensors Process**: Creates two threads:

    - RPM Thread: Generates data at 10Hz (100ms sleep).

    - Temp Thread: Generates data at 2Hz (500ms sleep).

**Shared Memory**: Threads write to the ```telemetry_monitor struct``` in RAM.

**Transmitter Process**: Wakes up when data is available, reads the packet, converts it to JSON (```{"val": ..., "ts": ...}```), and publishes it to ```fsae/car/RPM``` or ```fsae/car/TEMP```.

**Python Subscriber**: Receives the payload and prints it.

## ⚠️ Notes
The simulation runs automatically for 5 seconds before the parent process sends SIGKILL to child processes and cleans up resources.

The system uses localhost port 1883 for MQTT connections.