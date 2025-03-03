# ECE590 IoT Final Report | Spring 2024



---

## Video Demonstration

You can view the **video demonstration** of the system in action below:

[![IoT System Demonstration](https://img.youtube.com/vi/XBclVdYyS3Q/0.jpg)](https://www.youtube.com/watch?v=XBclVdYyS3Q)

> Replace `VIDEO_ID` with your actual YouTube video ID.

---

## Table of Contents

1. [Introduction and Motivation](#introduction-and-motivation)  
2. [System Design](#system-design)  
3. [System Implementation](#system-implementation)  
4. [Experiment and Evaluations](#experiment-and-evaluations)  
5. [Discussion and Conclusion](#discussion-and-conclusion)  

---

## Introduction and Motivation

### 1.1 Introduction  
This project focuses on developing an automated system capable of monitoring and dynamically adjusting the temperature and humidity levels to ensure the optimal preservation of sensitive products. The integration of security measures such as passcode protection against unauthorized access is also incorporated. In addition, the system is designed to respond to external conditions across various geographic locations and seasons. This document details the conception, design, implementation, and evaluation of an IoT solution that promises to redefine product safety and cost efficiency in business operations.

### 1.2 Motivation  
- **Enhanced Product Safety**: Many products, ranging from perishable foods to sensitive pharmaceuticals, require specific temperature and humidity conditions to maintain their quality and efficacy. An automated system can continuously monitor and adjust these conditions, significantly reducing the risk of product spoilage or damage due to environmental factors.
  
- **Security and Integrity**: By incorporating security measures such as passcode protection, the system ensures that environmental controls cannot be tampered with, thus maintaining the integrity of the logistics chain.
  
- **Adaptability and Scalability**: The ability to adjust parameters like humidity and temperature thresholds in real-time allows the system to handle a diverse range of products and adapt to varying external conditions, making it scalable across different geographic locations and seasons.

---

## System Design

### 2.1 Main Physical Components  
- **2 ULN2003 stepper motors** & drive boards control the 4 phases of the stepper motor sequentially to roll up/down the window based on humidity.
- **1 DHT 11 sensor** periodically tracks temperature and humidity.
- **PWM-controlled LED lights** for heat and brightness adjustments.
- **3D printed or recycled material for the shell cubic**.

### 2.2 System Wiring Diagram  
The wiring diagram shows the connection of the motors and the DHT11 sensor to the ESP32 board, with a detailed view of the GPIO pin configuration.

### 2.3 APP UI/UX Design  
The Android app has editable text areas for entering a passcode, and buttons for turning the system on/off and adjusting the humidity thresholds.

---

## System Implementation

### 3.1 DHT 11 Sensor  
The DHT11 sensor communicates with the ESP32 board using the 1-Wire protocol, where the system reads temperature and humidity data to control the gate position.

### 3.2 ULN2003 Stepper Motor  
The stepper motors are controlled based on the humidity, where the position of the gate is inversely proportional to the humidity level.

### 3.3 Android APP  
The Android app sends HTTP requests to the ESP32 board to control the system, including unlocking, adjusting the gate, and managing humidity thresholds.

---

## Experiment and Evaluations

### 4.1 Turn on / off Using Passcode  
The system is secured with a passcode, where only the correct passcode allows access to control the system.

### 4.2 Move Gate with Humidity  
The movement of the gate is recorded and adjusted based on the real-time humidity levels.

### 4.3 Changing PWM of LED  
LED brightness is dynamically adjusted using PWM based on the temperature values.

### 4.4 Change Min/Max Humidity Value  
Users can adjust the minimum and maximum humidity range using the app interface.

---

## Discussion and Conclusion

The system successfully met its goals, and some future improvements include:
1. Adding heat insulators to improve sensor accuracy.
2. Replacing the power supply with an uninterruptible power supply.
3. Enhancing the app with dynamic colors based on the current temperature.
4. Replacing LED lights with real heat lights for better temperature control.

---

## License

This project is for **educational and research purposes**. [Insert your preferred license here.]

## Acknowledgements

Thanks to my professors and peers for their support and feedback during the development of this project. Special thanks to the contributors of open-source libraries that helped in building this IoT solution.

