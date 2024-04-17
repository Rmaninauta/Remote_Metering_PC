#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>

const int chipSelect = 10; // Chip select pin for SD card
const int sensorPin = A0;  // Analog pin for voltage reading
const int sensorPinA1 = A1; // Additional analog pin for voltage reading
const int LED_PIN = 13;    // Built-in LED

// SoftwareSerial setup for ultrasonic sensor
const int RX_PIN = 2; // Connect this to TX of the sensor
const int TX_PIN = 3; // Not used, but required for SoftwareSerial initialization
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Initialize SoftwareSerial

unsigned char data[4]; // Buffer for storing received data from ultrasonic sensor
int distance;
float voltage, voltageA1;

const unsigned long measurementInterval = 100; // Measurement interval in milliseconds
unsigned long lastMeasurementTime = 0;

String lastDistance;
String pumpSensor, pressureVoltage;  // Renamed variables

void setup() {
    pinMode(LED_PIN, OUTPUT);
    wdt_enable(WDTO_8S); // Enable the watchdog timer to reset after 8 seconds of inactivity

    Serial.begin(9600);   // Start the hardware serial port for debugging
    mySerial.begin(9600); // Start SoftwareSerial for sensor communication

    if (!SD.begin(chipSelect)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized.");

    Serial.println("System ready");
}

void loop() {
    wdt_reset(); // Reset the watchdog timer

    if (millis() - lastMeasurementTime >= measurementInterval) {
        lastMeasurementTime = millis();
        readVoltage(sensorPin, voltage, pumpSensor); // Read and log the voltage from A0
        readVoltage(sensorPinA1, voltageA1, pressureVoltage); // Read and log the voltage from A1
        readDistance(); // Read distance from the ultrasonic sensor

        Serial.print(lastDistance);
        Serial.print(",");
        Serial.print(pumpSensor);
        Serial.print(",");
        Serial.print(pressureVoltage);
        Serial.println();

        // Save data to SD card
        File dataFile = SD.open("data.txt", FILE_WRITE);
        if (dataFile) {
            dataFile.print(lastDistance);
            dataFile.print(",");
            dataFile.print(pumpSensor);
            dataFile.print(",");
            dataFile.print(pressureVoltage);
            dataFile.println();
            dataFile.close();
        } else {
            Serial.println("error opening data.txt");
        }

        delay(2000); // Delay before the next data transmission
    }
}

void readDistance() {
    if (mySerial.available() > 10) {
        while (mySerial.available()) {
            mySerial.read();
            delay(2);
        }
    }

    unsigned long startTime = millis();
    while (mySerial.available() < 4 && millis() - startTime < 1000) {
    }

    if (mySerial.available() >= 4) {
        for (int i = 0; i < 4; i++) {
            data[i] = mySerial.read();
        }

        if (data[0] == 0xFF) {
            int sum = (data[0] + data[1] + data[2]) & 0xFF;
            if (sum == data[3]) {
                distance = (data[1] << 8) + data[2];
                lastDistance = String(distance);
            } else {
                Serial.println("error");
            }
        }
    }
}

void readVoltage(int pin, float &voltageVar, String &voltageStr) {
    int sensorValue = analogRead(pin);
    voltageVar = sensorValue * (5.0 / 1023.0);
    voltageStr = String(voltageVar, 3);
}
