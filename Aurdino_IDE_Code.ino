#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

// Wi-Fi Credentials
const char* ssid = "Airtel_sahi_0849";
const char* password = "air99772";

// Motor Pin Assignments
const int motorLeftPin1 = 25;   
const int motorLeftPin2 = 26;  
const int motorRightPin1 = 27; 
const int motorRightPin2 = 14; 

// Ultrasonic Sensor Pins
const int trigPin = 5;
const int echoPin = 18;

// Servo Pin for Soil Moisture Sensor Positioning
const int servoPin = 19;
Servo sensorServo;

// Soil Moisture Sensor and Pump Pins
const int moistureSensorPin = 34;  // Analog pin for moisture sensor
const int pumpPin = 32;            // Pin to control pump relay

// Detection and Stop Parameters
const float stopDistance = 15.0;   // Stop distance from the object in cm
const float maxDetectDistance = 100.0; // Maximum detection distance
const int moistureThreshold = 400; // Threshold for soil moisture (adjust as needed)

bool objectProcessed = false;  // Tracks if the current object has been watered

// Server setup
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // Initialize Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");

    // Print IP Address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // Set up Motor Pins
    pinMode(motorLeftPin1, OUTPUT);
    pinMode(motorLeftPin2, OUTPUT);
    pinMode(motorRightPin1, OUTPUT);
    pinMode(motorRightPin2, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Setup pump pin and servo
    pinMode(pumpPin, OUTPUT);
    digitalWrite(pumpPin, LOW);  // Pump off initially
    sensorServo.attach(servoPin);
    sensorServo.write(90);        // Initial position for servo is up

    // Start Server
    server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request){
        String direction;
        if (request->hasParam("direction", true)) {
            direction = request->getParam("direction", true)->value();
            controlMotor(direction);
            request->send(200, "text/plain", "Motor command received");
        } else {
            request->send(400, "text/plain", "Bad Request");
        }
    });

    server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request){
        float distance = measureDistance();
        String json = String("{\"distance\":") + distance + "}";
        request->send(200, "application/json", json);
    });

    server.begin();
}

void loop() {
    float distance = measureDistance();

    // Check if the object is within the threshold distance and hasn't been processed
    if (distance < stopDistance && !objectProcessed) {
        controlMotor("stop");
        performMoistureCheckAndWatering();
        objectProcessed = true;  // Mark object as processed
        findNextObject();        // Rotate to search for the next object
    }
    else if (distance < maxDetectDistance && !objectProcessed) {
        controlMotor("forward"); // Move towards object
    }
    else {
        controlMotor("stop");    // No object detected
        objectProcessed = false; // Reset for new object
    }
}

// Motor Control Function
void controlMotor(String direction) {
    if (direction == "forward") {
        digitalWrite(motorLeftPin1, HIGH);
        digitalWrite(motorLeftPin2, LOW);
        digitalWrite(motorRightPin1, HIGH);
        digitalWrite(motorRightPin2, LOW);
    } else if (direction == "backward") {
        digitalWrite(motorLeftPin1, LOW);
        digitalWrite(motorLeftPin2, HIGH);
        digitalWrite(motorRightPin1, LOW);
        digitalWrite(motorRightPin2, HIGH);
    } else if (direction == "left") {
        digitalWrite(motorLeftPin1, LOW);
        digitalWrite(motorLeftPin2, HIGH);
        digitalWrite(motorRightPin1, HIGH);
        digitalWrite(motorRightPin2, LOW);
    } else if (direction == "right") {
        digitalWrite(motorLeftPin1, HIGH);
        digitalWrite(motorLeftPin2, LOW);
        digitalWrite(motorRightPin1, LOW);
        digitalWrite(motorRightPin2, HIGH);
    } else if (direction == "stop") {
        digitalWrite(motorLeftPin1, LOW);
        digitalWrite(motorLeftPin2, LOW);
        digitalWrite(motorRightPin1, LOW);
        digitalWrite(motorRightPin2, LOW);
    }
}

// Measure Distance with Ultrasonic Sensor
float measureDistance() {
    long duration;
    float distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration * 0.034) / 2;
    return distance;
}

// Function to Check and Water based on Soil Moisture
void performMoistureCheckAndWatering() {
    Serial.println("Performing moisture check...");
    sensorServo.write(0);      // Lower the moisture sensor
    delay(1000);                // Wait for sensor to stabilize

    int moistureLevel = analogRead(moistureSensorPin);
    Serial.print("Soil Moisture Level: ");
    Serial.println(moistureLevel);

    // Start checking moisture level and activate pump if necessary
    if (moistureLevel < moistureThreshold) {
        Serial.println("Moisture insufficient, starting pump...");
        digitalWrite(pumpPin, HIGH);  // Turn on the pump
        delay(2000);                  // Run pump for a short period
    } else {
        Serial.println("Moisture level sufficient, stopping pump.");
    }
    
    // Check moisture level again after watering
    moistureLevel = analogRead(moistureSensorPin);  
    Serial.print("Updated Soil Moisture Level: ");
    Serial.println(moistureLevel);

    if (moistureLevel < moistureThreshold) {
        Serial.println("Insufficient moisture after watering, running pump again...");
        digitalWrite(pumpPin, HIGH);  // Turn on the pump again
        delay(2000);                  // Run pump for a short period
    }
    
    digitalWrite(pumpPin, LOW);      // Turn off the pump after watering
    sensorServo.write(90);            // Lift the sensor back up
    delay(500);
    markObject();                    // Mark object as watered
}

// Function to Mark the Object
void markObject() {
    Serial.println("Object marked as processed.");
    // Servo action or other mechanism can go here if marking physically
}

// Function to Find the Next Object
void findNextObject() {
    Serial.println("Searching for next object...");
    controlMotor("right");   // Rotate to look for the next object
    delay(1000);             // Rotate for some time
    controlMotor("stop");    // Stop after rotation
    delay(500);
}