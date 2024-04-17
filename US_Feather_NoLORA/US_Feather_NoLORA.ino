#include <SPI.h>
#include <SD.h>

// SD card and LED definitions
const int SD_CS = 4;
const int LED_PIN = 13;

unsigned char data[4] = {};
int distance; // Changed to int to match your working example
const unsigned long sensorReadInterval = 500;
unsigned long lastSensorReadTime = 0;
char logFilename[13];

void setup() {
    pinMode(LED_PIN, OUTPUT);
    if (!SD.begin(SD_CS)) {
        flashLED();
    }

    int fileNumber = 0;
    do {
        sprintf(logFilename, "log%04d.txt", fileNumber++);
    } while (SD.exists(logFilename));
    logData("Starting setup...");
    Serial1.begin(9600);
}

void loop() {
    if (millis() - lastSensorReadTime >= sensorReadInterval) {
        lastSensorReadTime = millis();

        while (Serial1.available() > 0) { // Clear the serial buffer
            Serial1.read();
        }

        while (Serial1.available() < 4) { // Wait for fresh data
            delay(10);
        }

        for (int i = 0; i < 4; i++) { // Read fresh data
            data[i] = Serial1.read();
        }

        if (data[0] == 0xff) {
            int sum = (data[0] + data[1] + data[2]) & 0x00FF;
            if (sum == data[3]) {
                distance = (data[1] << 8) + data[2]; // Calculate distance
                if (distance > 30) {
                    char message[50];
                    sprintf(message, "Distance = %d cm", distance / 10);
                    logData(message);
                } else {
                    logData("Below the lower limit");
                }
            } else {
                logData("Checksum ERROR");
            }
        }
    }
}

void logData(const char* message) {
    File logFile = SD.open(logFilename, FILE_WRITE);
    if (!logFile) {
        flashLED();
        return;
    }
    logFile.println(message);
    logFile.close();
}

void flashLED() {
    while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(10);
        digitalWrite(LED_PIN, LOW);
        delay(10);
    }
}
