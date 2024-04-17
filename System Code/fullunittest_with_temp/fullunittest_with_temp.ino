#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Si7021.h>
#include <EEPROM.h>  // Include the EEPROM library

const int chipSelect = 10;
const int sensorPin = A0;
const int sensorPinA1 = A1;
const int LED_PIN = 13;

const int RX_PIN = 2;
const int TX_PIN = 3;
SoftwareSerial mySerial(RX_PIN, TX_PIN);

unsigned char data[4];
int distance;
float voltage, voltageA1;
String lastDistance;
String pumpSensor, pressureVoltage, tempHumidity;

Adafruit_Si7021 sensor = Adafruit_Si7021();

String dataFileName;

const unsigned long measurementInterval = 100;
unsigned long lastMeasurementTime = 0;

int fileIndexAddress = 0; // Address in EEPROM to store file index

void setup() {
    pinMode(LED_PIN, OUTPUT);
    wdt_enable(WDTO_8S);

    Serial.begin(9600);
    mySerial.begin(9600);
    Wire.begin();

    if (!sensor.begin()) {
        Serial.println("Si7021 sensor not found!");
        while (true);
    }

    if (!SD.begin(chipSelect)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    int fileIndex = EEPROM.read(fileIndexAddress) + 1; // Read the current index and increment
    EEPROM.write(fileIndexAddress, fileIndex % 256);  // Store the new index (wrap around at 255)

    char filename[20];
    sprintf(filename, "DATA%03d.TXT", fileIndex);
    dataFileName = String(filename);

    File dataFile = SD.open(dataFileName, FILE_WRITE);
    if (dataFile) {
        Serial.println("Data file created: " + dataFileName);
        dataFile.close();
    } else {
        Serial.println("Failed to create data file.");
    }

    Serial.println("SD card and sensors initialized.");
    Serial.println("System ready");
}

void loop() {
    wdt_reset();

    if (millis() - lastMeasurementTime >= measurementInterval) {
        lastMeasurementTime = millis();
        readVoltage(sensorPin, voltage, pumpSensor);
        readVoltage(sensorPinA1, voltageA1, pressureVoltage);
        readDistance();
        readSi7021(tempHumidity);

        Serial.print("Distance Sensor: ");
        Serial.print(lastDistance);
        Serial.print("cm, Current Sensor: ");
        Serial.print(pumpSensor);
        Serial.print("V, Pressure Transducer: ");
        Serial.print(pressureVoltage);
        Serial.print("V, ");
        Serial.println(tempHumidity);

        File dataFile = SD.open(dataFileName, FILE_WRITE);
        if (dataFile) {
            dataFile.print("Distance Sensor: ");
            dataFile.print(lastDistance);
            dataFile.print("cm, Current Sensor: ");
            dataFile.print(pumpSensor);
            dataFile.print("V, Pressure Transducer: ");
            dataFile.print(pressureVoltage);
            dataFile.print("V, ");
            dataFile.println(tempHumidity);
            dataFile.close();
        } else {
            Serial.println("Error opening data file");
        }

        delay(2000);
    }
}


void readSi7021(String &tempHumStr) {
    float tempC = sensor.readTemperature();        // Read temperature in Celsius
    float tempF = tempC * 9.0 / 5.0 + 32;          // Convert Celsius to Fahrenheit
    float hum = sensor.readHumidity();             // Read humidity
    tempHumStr = "Temp: " + String(tempF, 1) + "F, Hum: " + String(hum, 1) + "%";
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
