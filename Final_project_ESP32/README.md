ESP32 Automated Window Control System
Introduction
This project is designed to automatically adjust a window based on humidity levels using an ESP32 microcontroller. It includes Wi-Fi connectivity to allow for remote monitoring and control via a simple web server. The system reads humidity and temperature data from a DHT11 sensor and adjusts the window's position using a motor controlled via GPIO pins. Additionally, it features an LED brightness adjustment based on the humidity levels.

Features
Wi-Fi connectivity to access and control the system remotely.
Real-time humidity and temperature monitoring.
Automated window adjustment based on predefined humidity thresholds.
Web server interface for manual control and monitoring.
LED feedback based on current humidity level.
Hardware Requirements
ESP32 Development Board
DHT11 Temperature and Humidity Sensor
Motor for window control
LEDs for feedback
Basic resistors, wires, and breadboard
Software Requirements
ESP-IDF v4.x

Required libraries:
freertos
esp_wifi
esp_http_client
cJSON
lvgl for optional UI components


Installation
1. Setup ESP-IDF
Ensure you have ESP-IDF installed and set up on your system. Follow the instructions from the official Espressif documentation.

2. Clone the Repository
Clone the project repository to your local machine using:

bash
Copy code

3. Configuration
Configure your Wi-Fi credentials and other settings through menuconfig:

bash
Copy code
idf.py menuconfig
Navigate to "Example Configuration" to set your Wi-Fi SSID and password.

4. Build and Flash
Compile and flash the firmware to your ESP32:

bash
Copy code
idf.py build
idf.py -p [YOUR_SERIAL_PORT] flash
Usage
Once the device starts, it will connect to the specified Wi-Fi network. You can access the web server at the IP address displayed in the serial output.

Endpoints
/on - Turns on the system
/off - Turns off the system
/Password - Displays the current system password
/Unlock - Unlocks the system
/Lock - Locks the system
/Min and /Max - Get or set the minimum and maximum humidity levels

Ensure your Android phone is connected to the same WIFI as ESP32