# Smart-Sprinkler-IoT-System-for-Plants

###  B.Tech Final Year Project
The Smart Sprinkler IoT System for Plants is an Internet of Things (IoT)-enabled robotic system that combines machine learning-based object detection, embedded systems, and real-time sensor data processing to automate plant irrigation. The designed system is able to tackle important problems, including over-irrigation, water leakage, and the necessity of manual works.

Modern agriculture is challenged by critical issues such as water scarcity, labour-intensive practices and deficiencies in conventional irrigation infrastructure. Conventional irrigation systems usually suffer from water loss, inefficient water allocation and heavy reliance on manual labour, therefore incur high operating costs, lower production efficiency and environmental damage. These constraints underscore the critical need for novel solutions that maximize efficient use of resources and minimize the involvement of human operators.

### There are two main subsystems inside the system:

1. An ESP32-based mobile robot is a wheeled robotic platform that has a water pump, servo-mounted soil moisture probe, DC motors and an ultrasonic distance sensor. It uses obstacle avoidance to function independently and connects to a central vision system via WiFi. 

2. Computer Vision & Control System: YOLOv8 (You Only Look Once) is a Python application that uses an ESP32-CAM live video feed to recognize plants in real time. The system gives navigation commands to the robot, processes the locations of detected plants and uses HTTP requests to check the moisture content of the soil.

## Hardware Components Specifications
1. ESP32-WROOM-32 Microcontroller:
<img width="307" height="307" alt="image" src="https://github.com/user-attachments/assets/7324732d-fcc4-432a-af2d-2e9399c4cfb1" />

Description: The ESP32 serves as the brain of the system, integrating Wi-Fi/Bluetooth for communication. It features a dual-core Xtensa LX6 processor (240MHz), 34 GPIO pins (12 ADC, 2 DAC), and ultra-low power consumption (5μA in deep sleep). Manages motor control, sensor data acquisition, and HTTP server hosting.

2. ESP32-CAM (OV2640 Camera):
<img width="185" height="171" alt="image" src="https://github.com/user-attachments/assets/81b1f1ec-c32c-4135-a12a-a371cfce1d7c" />

Description: Combines an ESP32-S chip with an OV2640 2MP camera. Streams video at 5–10 FPS (1600×1200 resolution) over Wi-Fi for real-time plant detection using YOLOv8. Requires external 5V power due to high current draw (180mA).

3. HC-SR04 Ultrasonic Sensor:
<img width="206" height="133" alt="image" src="https://github.com/user-attachments/assets/5c7ab835-f0af-41b4-81fa-e101143042ef" />

Description: Measures distances (2cm–4m) with ±3mm accuracy. Uses 40kHz sound waves triggered by a 10μs pulse (5V). Critical for obstacle avoidance and stopping the robot 15cm from plants.

4. Capacitive Soil Moisture Sensor:
<img width="214" height="143" alt="image" src="https://github.com/user-attachments/assets/30f666fd-1139-4812-9d4e-1d4461e0c822" />

Description: Provides analog output (0–4095) proportional to soil water content. Corrosion-resistant (unlike resistive sensors). Calibrated to trigger watering at <400 (dry soil) and stop at >1500 (saturated).

5. L298N Motor Driver:
<img width="192" height="165" alt="image" src="https://github.com/user-attachments/assets/4e602a7d-902d-4fbb-b57a-f5edc877fd0e" />

Description: Dual H-bridge driver controlling two 6V–12V DC motors (500mA peak per channel). Accepts 5V logic inputs from ESP32 for forward/backward/left/right movements.

6. SG90 Servo Motor: 
<img width="234" height="197" alt="image" src="https://github.com/user-attachments/assets/04748cc1-ca40-4a5c-b70d-c66f87087500" />
                         
Description: 180° rotation servo (1.8 kg-cm torque) to lower/raise the soil moisture sensor. Controlled via PWM (50Hz, 500–2500μs pulse width).

7. 5V Submersible Water Pump:
<img width="141" height="142" alt="image" src="https://github.com/user-attachments/assets/accff6ba-8355-4c86-ab6e-e527c1d5a26b" />

Description: Mini DC pump (1L/min flow rate) activated via a relay. Runs for 2-second pulses when soil moisture is below threshold. Protected by a flyback diode (1N4007).

8. 18650 Li-ion Battery Pack:
<img width="148" height="144" alt="image" src="https://github.com/user-attachments/assets/be49dc14-51da-4d36-bb09-d87e6ad887f8" />

Description: 7.4V 3000mAh rechargeable pack powers the system for 2.5 hours (active use). Paired with a TP4056 charger and buck converter (7.4V→5V/3.3V).

## Software Architecture
#### A. Robot Firmware (ESP32):


•	Web Server: AsyncHTTP server for receiving commands.

•	APIs:

 POST /control: Motor commands (forward/backward/left/right/stop).

 GET /distance: Ultrasonic distance feedback.

 POST /pump: Pump control (ON/OFF).
 


#### B. Computer Vision (Python):
•	Libraries: OpenCV, Ultralytics (YOLOv8), Requests.

•	Workflow:

1.	Capture ESP32-CAM stream.
   
2.	Run YOLOv8 inference on each frame.
	
3.	If plant detected:
   
     Send STOP command to ESP32.
   
     Trigger moisture check.
  	
4.	If no plant: Continue navigation.


## System Workflows
#### A. Autonomous Navigation:
1.	Step 1: Robot moves forward (DC motors).
2.	Step 2: Ultrasonic sensor scans for obstacles (<15cm → stop).
3.	Step 3: ESP32-CAM detects plants → YOLOv8 confirms.
4.	Step 4: If plant is valid, initiate moisture check.

#### B. Smart Watering:
1.	Step 1: Servo lowers moisture sensor into soil.
2.	Step 2: Analog reading compared to threshold (default: 400).
3.	Step 3: If dry, pump runs for 2 seconds.
4.	Step 4: Servo raises sensor → robot resumes search.

#### C. Fault Handling:
•	No Plant Detected: Robot rotates (15° increments) to search.

•	Low Battery: ESP32 enters deep sleep (solar recharge if available).

•	Sensor Failure: HTTP alert sent to monitoring system.


## Project Prototype

<img width="975" height="553" alt="image" src="https://github.com/user-attachments/assets/b17b9fc7-ce22-46c9-b80b-97c7384cb106" />
