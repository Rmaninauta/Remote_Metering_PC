#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h> // Include the watchdog timer library

// SD card and sensor definitions
const int SD_CS = 4;
const int sensorPin = A0;  // Analog pin for voltage reading
const int LED_PIN = 13;    // Built-in LED

// Variables for distance measurement and voltage reading
unsigned char data[4] = {};
int distance;
float voltage;
const unsigned long measurementInterval = 1200000; // 20 minutes in milliseconds
unsigned long lastMeasurementTime = 0;

// File handling variables
int fileCounter = 0;
char logFilename[15];

// Variables to store last read data
String lastDistance;
String lastVoltage;

void setup() {
    wdt_enable(WDTO_8S); // Enable the watchdog timer to reset after 8 seconds of inactivity
    pinMode(LED_PIN, OUTPUT);

    // Initialize SD Card
    if (!SD.begin(SD_CS)) {
        flashLED(); // Flash LED rapidly due to SD card failure
    }

    // Find a new file name to avoid overwriting
    do {
        sprintf(logFilename, "log_%d.txt", fileCounter++);
    } while (SD.exists(logFilename));

    File dataFile = SD.open(logFilename, FILE_WRITE);
    if (dataFile) {
        dataFile.println("SD card init OK!");
        dataFile.close();
    } else {
        flashLED(); // Flash LED rapidly due to file creation failure
    }

    Serial.begin(9600);
    logData("Setup complete");
}

void loop() {
    wdt_reset(); // Reset the watchdog timer

    if (millis() - lastMeasurementTime >= measurementInterval) {
        lastMeasurementTime = millis();
        performMeasurements();
    }

    if (Serial) {
        // Format the string to fit the 16x2 LCD display
        char buffer[32]; // Buffer to hold the formatted string
        snprintf(buffer, sizeof(buffer), "~%0.1fcm, P%0.1fV", lastDistance, lastVoltage);
        
        Serial.println(buffer);
        }

}

void performMeasurements() {
    // Distance measurement
    if (Serial.available() >= 4) {
        for (int i = 0; i < 4; i++) {
            data[i] = Serial.read();
        }

        if (data[0] == 0xff) {
            int sum = (data[0] + data[1] + data[2]) & 0x00FF;
            if (sum == data[3]) {
                distance = (data[1] << 8) + data[2];
                lastDistance = String(distance / 10);
                logData("Distance: " + lastDistance + " cm");
            } else {
                logData("Checksum ERROR");
            }
        }
    }

    // Voltage reading
    int sensorValue = analogRead(sensorPin);
    voltage = sensorValue * (5.0 / 1023.0); // Assuming a 5V reference
    lastVoltage = String(voltage, 3);
    logData("Voltage: " + lastVoltage + " V");
}

void logData(String message) {
    File dataFile = SD.open(logFilename, FILE_WRITE);
    if (dataFile) {
        dataFile.println(message);
        dataFile.close();
    } else {
        flashLED(); // Flash LED rapidly due to file opening failure
    }
}

void flashLED() {
    while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(100); // LED on for 100ms
        digitalWrite(LED_PIN, LOW);
        delay(100); // LED off for 100ms
    }
}
