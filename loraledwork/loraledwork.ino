#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  6  // "D"
#define LED_PIN_REPLY 12  // LED to indicate a reply is received
#define LED_PIN_NO_REPLY 13  // LED to indicate no reply or failure

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
 // while (!Serial) delay(1);
  delay(100);

  pinMode(LED_PIN_REPLY, OUTPUT);
  pinMode(LED_PIN_NO_REPLY, OUTPUT);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop() {
  delay(1000); // Wait 1 second between transmits
  Serial.println("Transmitting..."); // Send a message to rf95_server

  char radiopacket[20] = "Hello World #      ";
  itoa(packetnum++, radiopacket+13, 10);
  Serial.print("Sending "); Serial.println(radiopacket);
  radiopacket[19] = 0;

  Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)radiopacket, 20);

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();

  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  if (rf95.waitAvailableTimeout(1000)) { // Wait for a reply
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      digitalWrite(LED_PIN_REPLY, HIGH);  // Indicate reply received
      delay(200); // Short delay to visually indicate reception
      digitalWrite(LED_PIN_REPLY, LOW);
    } else {
      Serial.println("Receive failed");
      digitalWrite(LED_PIN_NO_REPLY, HIGH);  // Indicate failure
      delay(200); // Short delay to visually indicate failure
      digitalWrite(LED_PIN_NO_REPLY, LOW);
    }
  } else {
    Serial.println("No reply, is there a listener around?");
    digitalWrite(LED_PIN_NO_REPLY, HIGH);  // Indicate no reply
    delay(200); // Short delay to visually indicate no reply
    digitalWrite(LED_PIN_NO_REPLY, LOW);
  }
}
