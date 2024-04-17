#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address if necessary

void setup() {
  Serial.begin(9600);  // Start serial communication at 9600 baud
  lcd.init();          // Initialize the LCD
  lcd.backlight();     // Turn on the backlight
}

void loop() {
  if (Serial.available() > 0) {
    // Read the incoming data until a newline character is received
    String data = Serial.readStringUntil('\n');

    // Parse the data into distance and voltage
    int commaIndex = data.indexOf(',');
    String distance = data.substring(0, commaIndex);
    String voltage = data.substring(commaIndex + 1);

    // Display the distance on the top line of the LCD
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print(" cm");

    // Display the voltage on the bottom line of the LCD
    lcd.setCursor(0, 1);
    lcd.print("Volt: ");
    lcd.print(voltage);
    lcd.print(" V");
  }
}
