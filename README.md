# ECE590 IoT Final Report | Spring 2024

## Video Demonstration

You can view the **video demonstration** of the system in action below:



[Download the system demo video](./Demo.mov)



## Introduction and Motivation

This project focuses on developing an automated system capable of monitoring and dynamically adjusting the temperature and humidity levels to ensure the optimal preservation of sensitive products. The integration of security measures such as passcode protection against unauthorized access is also incorporated. The system is designed to respond to external conditions across various geographic locations and seasons, offering product safety and cost efficiency.

### Motivation  
- **Enhanced Product Safety**: Continuously monitor and adjust conditions to reduce risks like spoilage or damage.
- **Security**: Incorporates passcode protection to maintain the integrity of environmental controls.
- **Adaptability**: Adjusts to varying external conditions across different locations and seasons.

---

## System Design

### 2.1 Main Physical Components  
- **2 ULN2003 stepper motors** & drive boards control the gate position based on humidity.
- **1 DHT 11 sensor** to track temperature and humidity.
- **PWM-controlled LED lights** to simulate heat for adjusting the temperature.

### 2.2 System Wiring Diagram  
The wiring connects the motors, DHT11 sensor, and ESP32 board. Stepper motors are powered by batteries and controlled via GPIO pins.

### 2.3 APP UI/UX Design  
The Android app allows users to input a passcode, control the system, and adjust humidity thresholds via buttons and text boxes.

---

## System Implementation

### 3.1 DHT 11 Sensor  
Uses the 1-Wire protocol to communicate with the ESP32 board to monitor environmental data like temperature and humidity.

### 3.2 ULN2003 Stepper Motor  
Motor controls are based on humidity, with GPIO pins driving the motor to move the gate.

### 3.3 Android APP  
The app sends HTTP requests to control the system based on the passcode, adjust the gate, and change the humidity limits.

---

## Experiment and Evaluations

### 4.1 Turn on / off Using Passcode  
The system locks and unlocks based on a passcode, allowing secure control over the system.

### 4.2 Move Gate with Humidity  
The gate position changes dynamically based on the humidity level, controlled by the motors.

### 4.3 Changing PWM of LED  
LED brightness is adjusted based on the temperature to simulate heating for the environment.

### 4.4 Change Min/Max Humidity Value  
Allows users to dynamically change the upper and lower humidity values to control the gate’s position.

---

## Discussion and Conclusion

The project successfully met its design goals, with future improvements including:
1. Adding insulation to improve sensor accuracy.
2. Replacing power supply with an uninterruptible power source for enhanced stability.
3. Adding dynamic button color changes based on temperature in the app.
4. Replacing LED lights with real heat lamps for better temperature control.

---

## License

This project is for **educational and research purposes**. [Insert your preferred license here.]

## Acknowledgements

Thanks to my professors and peers for their support and feedback during the development of this project.
