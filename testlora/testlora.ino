#include <RH_RF95.h>

#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  6  // "D"
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission


void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
 // while (!Serial); // Wait for the serial port to connect. Needed for native USB
  delay(1000); // Wait a bit for serial console
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa radio init failed");
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

void loop() {
  Serial.println("Transmitting..."); 

  char radiopacket[20] = "Hello World #      ";
  itoa(packetnum++, radiopacket+13, 10);
  Serial.print("Sending "); Serial.println(radiopacket);

  rf95.send((uint8_t *)radiopacket, 20);
  rf95.waitPacketSent();

  Serial.println("Waiting for reply...");
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(1000)) {
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      // Adjusted RSSI check for demonstration purposes
      digitalWrite(13, rf95.lastRssi() > -50 ? HIGH : LOW);  // RSSI is negative. Closer to 0 means stronger signal
      digitalWrite(12, rf95.lastRssi() <= -50 ? HIGH : LOW);
    } else {
      Serial.println("Receive failed");
      digitalWrite(13, HIGH);  // Error state
      digitalWrite(12, HIGH);  // Error state
    }
  } else {
    Serial.println("No reply, is there a listener around?");
    digitalWrite(13, LOW);  // Indicates no reply
    digitalWrite(12, HIGH);  // Indicates no reply
  }
  delay(1000); // Wait 1 second before next transmit
}
