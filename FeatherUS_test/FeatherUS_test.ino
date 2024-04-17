#include <SPI.h>
#include <RH_RF95.h>

// LoRa definitions
#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  6  // "D"
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

unsigned char data[4] = {};
float distance;
unsigned long lastSensorReadTime = 0;
const unsigned long sensorReadInterval = 100; // 20 minutes in milliseconds

void setup() {
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    Serial.begin(9600);  // Communication over USB
    Serial1.begin(9600);  // Communication with the sensor on Serial1

    delay(100);

    Serial.println("Arduino LoRa TX Test!");

    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        while (1); // Stop execution if init fails
    }
    Serial.println("LoRa radio init OK!");

    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1); // Stop execution if set frequency fails
    }
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
    
    rf95.setTxPower(23, false);
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastSensorReadTime >= sensorReadInterval) {
        lastSensorReadTime = currentMillis; // Update the last read time

        if (Serial1.available() >= 4) {
            for (int i = 0; i < 4; i++) {
                data[i] = Serial1.read();
                // Comment out the following line in the final version to reduce serial output delay
                // Serial.print("Data["); Serial.print(i); Serial.print("]: "); Serial.println(data[i], HEX);
            }

            if (data[0] == 0xff) {
                int sum = (data[0] + data[1] + data[2]) & 0x00FF;
                if (sum == data[3]) {
                    distance = (data[1] << 8) + data[2];
                    Serial.print("distance=");
                    Serial.print(distance / 10);
                    Serial.println(" cm");
                    
                } else {
                    Serial.println("Checksum ERROR");
                }
                delay(100);
            }
        }
    }
}
