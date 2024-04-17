void setup() {
    Serial.begin(9600);   // Start the hardware serial port for debugging
    Serial1.begin(9600);  // Start Serial1 for sensor communication
    Serial.println("Distance sensor ready");
}

void loop() {
    unsigned char data[4];
    int distance;

    if (Serial1.available() >= 4) { // Check if at least 4 bytes are available from the sensor
        for (int i = 0; i < 4; i++) {
            data[i] = Serial1.read(); // Read the incoming byte
        }

        if (data[0] == 0xFF) { // Check for the start byte if required by your sensor's protocol
            int sum = (data[0] + data[1] + data[2]) & 0xFF; // Calculate the sum for checksum
            if (sum == data[3]) { // Verify checksum if required by your sensor's protocol
                distance = (data[1] << 8) + data[2]; // Calculate the distance
                Serial.println("Distance: " + String(distance) + " cm"); // Output the distance through hardware serial to the Serial Monitor
            } else {
                Serial.println("Checksum error"); // Checksum mismatch
            }
        }
    }
}

