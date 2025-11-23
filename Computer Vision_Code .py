import cv2
import numpy as np
from ultralytics import YOLO
import requests
import time
import threading

# Initialize YOLO model with custom weights
model = YOLO('best (1).pt')  # Replace with your trained model's path

# Define custom class names
classNames = ['Plants']

# Set the ESP32-CAM's streaming URL and robot IP
esp32_camera_ip = ""
esp32_robot_ip = ""

# Set confidence threshold
confidence_threshold = 0.5

# Soil moisture threshold for watering
moisture_threshold = 400  # Adjust based on your sensor calibration
desired_stopping_distance = 15  # Distance to stop before the plant

# Initialize video capture from the ESP32-CAM
cap = cv2.VideoCapture(esp32_camera_ip)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)  # Set frame width
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)  # Set frame height
cap.set(cv2.CAP_PROP_FPS, 15)  # Set lower frame rate

# Function to control the servo
def lower_sensor():
    requests.post(esp32_robot_ip + "/sensor", data={"action": "lower"})

def raise_sensor():
    requests.post(esp32_robot_ip + "/sensor", data={"action": "raise"})

# Function to get the distance from the ultrasonic sensor
def get_distance():
    try:
        response = requests.get(esp32_robot_ip + "/distance")
        response.raise_for_status()
        return response.json().get('distance', None)
    except requests.RequestException as e:
        print(f"Error retrieving distance: {e}")
        return None

# Function to check soil moisture level
def check_soil_moisture():
    try:
        sensor_response = requests.get(esp32_robot_ip + "/sensor_values")
        sensor_response.raise_for_status()
        data = sensor_response.json()
        return data['moisture']
    except requests.RequestException as e:
        print(f"Failed to get sensor data: {e}")
        return None

# Function to control robot movement
def control_movement(direction):
    requests.post(esp32_robot_ip + "/control", data={"direction": direction})

# Function to avoid obstacles
def avoid_obstacle():
    distance = get_distance()
    if distance is not None and distance < desired_stopping_distance:
        control_movement("stop")
        print(f"Obstacle detected. Stopping at distance: {distance} cm.")
        time.sleep(1)  # Wait for a moment before continuing
    else:
        control_movement("forward")  # Move forward if no obstacle

# Function to perform object detection and control watering
def object_detection_and_watering():
    while True:
        ret, img = cap.read()
        if not ret:
            print("Failed to capture image from ESP32-CAM. Retrying...")
            time.sleep(1)  # Wait before retrying
            continue  # Skip to the next iteration of the loop

        results = model(img, stream=True)
        found_plant = False  # Flag to check if a plant object is found

        for r in results:
            boxes = r.boxes
            for box in boxes:
                x1, y1, x2, y2 = box.xyxy[0].int()  # Ensure these are integers
                conf = round(box.conf[0].item(), 2)

                if conf > confidence_threshold:
                    cls = int(box.cls[0])
                    label = f'{classNames[cls]} {conf:.2f}'

                    # Draw bounding box with integer coordinates
                    cv2.rectangle(img, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                    cv2.putText(img, label, (max(0, int(x1)), max(35, int(y1))),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

                    # Stop near the plant
                    control_movement("stop")
                    print("Stopping near the plant.")
                    found_plant = True  # Mark that we found a plant

                    # Lower the soil moisture sensor
                    lower_sensor()
                    time.sleep(1)  # Give it time to lower

                    # Check soil moisture level
                    moisture = check_soil_moisture()

                    # Display moisture levels
                    cv2.putText(img, f'Soil Moisture: {moisture}', (10, 60),
                                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

                    # Control the pump based on moisture levels
                    if moisture is not None and moisture < moisture_threshold:
                        requests.post(esp32_robot_ip + "/pump", data={"state": "on"})
                        cv2.putText(img, "Pump: ON", (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1,
                                    (0, 0, 255), 2)
                        time.sleep(5)  # Pump on for a duration
                        requests.post(esp32_robot_ip + "/pump", data={"state": "off"})
                        cv2.putText(img, "Pump: OFF", (10, 120), cv2.FONT_HERSHEY_SIMPLEX, 1,
                                    (0, 255, 0), 2)

                    # Raise the moisture sensor back to zero position
                    raise_sensor()
                    time.sleep(1)  # Give it time to raise

        # If no plant was found, avoid obstacles
        if not found_plant:
            avoid_obstacle()

        # Display the frame
        cv2.imshow("Object Detection", img)

        # Exit on 'q' key press
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Start the object detection thread
detection_thread = threading.Thread(target=object_detection_and_watering)
detection_thread.start()

# Main loop to avoid obstacles
while True:
    avoid_obstacle()
    time.sleep(0.1)  # Small delay to prevent overwhelming the server

# Release the capture and close windows
cap.release()
cv2.destroyAllWindows