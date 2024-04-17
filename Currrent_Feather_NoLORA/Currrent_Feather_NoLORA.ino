#include <SPI.h>
#include <SD.h>

// SD card definitions
const int SD_CS = 4;

// Analog pin for voltage reading
const int sensorPin = A0;

// LED definitions
const int LED_PIN = 13; // Assuming you're using built-in LED

int fileCounter = 0; // File counter for unique filename

void setup() {
    pinMode(LED_PIN, OUTPUT); // Set LED pin as output

    // Initialize SD Card
    if (!SD.begin(SD_CS)) {
        flashLED(); // Flash LED rapidly due to SD card failure
    }

    // Create a new log file
    char filename[15];
    sprintf(filename, "log_%d.txt", fileCounter++);
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.close();
        logData("SD card init OK! Created file: " + String(filename));
    } else {
        logData("SD card init OK! Failed to create file: " + String(filename));
        flashLED(); // Flash LED rapidly due to file creation failure
    }
}

void loop() {
    // Read the voltage
    int sensorValue = analogRead(sensorPin);
    float voltage = sensorValue * (3.3 / 1023.0); // Convert the reading to voltage

    // Log voltage data to SD card
    char filename[15];
    sprintf(filename, "log_%d.txt", fileCounter - 1);
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.print("Voltage: ");
        dataFile.print(voltage);
        dataFile.println(" V");
        dataFile.close();
        //logData("Measured Voltage: " + String(voltage) + " V");
    } else {
        logData("Error opening file on SD card");
        flashLED(); // Flash LED rapidly due to file opening failure
    }

    delay(600000); // Wait for 10 seconds before the next read
}

void logData(String message) {
    char filename[15];
    sprintf(filename, "log_%d.txt", fileCounter - 1);
    File logFile = SD.open(filename, FILE_WRITE);
    if (logFile) {
        logFile.println(message);
        logFile.close();
    }
}

void flashLED() {
    while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(10); // LED on for 10ms
        digitalWrite(LED_PIN, LOW);
        delay(10); // LED off for 10ms
    }
}
