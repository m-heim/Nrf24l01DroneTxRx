#include <RF24_config.h>
#include <printf.h>
#include <RF24.h>
#include <nRF24L01.h>

#define MOTOR_1 3
#define MOTOR_2 5
#define MOTOR_3 6
#define MOTOR_4 9
#define SENDER 0
#define X_LEFT A4
#if SENDER
#define CE_PIN 9
#define CSN_PIN 10
#else 
#define CE_PIN 7
#define CSN_PIN 8
#endif
#define PAYLOAD_LENGTH 4

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
int radioNumber = SENDER;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
uint8_t payload[PAYLOAD_LENGTH];

void all(int speed) {
  analogWrite(MOTOR_1, speed);
  analogWrite(MOTOR_2, speed);
  analogWrite(MOTOR_3, speed);
  analogWrite(MOTOR_4, speed);
}

void setup() {
  Serial.begin(115200);
  if (Serial) {
    Serial.println("Serial initialized");
  }
  if (!radio.begin()) {
    pinMode(LED_BUILTIN, OUTPUT);
    if (Serial) {
      Serial.println("Device is not responding");
    }
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  } else {
    if (Serial) {
      Serial.println("Nrf radio successfully connected");
    }
  }
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(payload));
  if (!SENDER) {
    pinMode(10, OUTPUT); // set pin 10 for output, necessary for spi
    pinMode(MOTOR_1, OUTPUT);
    pinMode(MOTOR_2, OUTPUT);
    pinMode(MOTOR_3, OUTPUT);
    pinMode(MOTOR_4, OUTPUT);
    radio.openReadingPipe(1, address[0]);
    radio.startListening();
  } else {
    pinMode(X_LEFT, INPUT);
    radio.openWritingPipe(address[0]);
    radio.stopListening();
  }
  if (Serial) {
    Serial.println("Device set up successfully");
  }
}

void loop() {
  if (SENDER) {
    uint8_t raw = analogRead(X_LEFT);
    uint8_t val = raw - 505;
    val = -val;
    val = val / 2;
    if (val <= 0) {
      val = 0;
    }
    payload[0] = val;
    bool report = radio.write(&payload, sizeof(payload));
    if (Serial) {
      Serial.println(val, DEC);
      if (report) {
        Serial.println("Message was successfully transmitted");
      } else {
        Serial.println("No ack from receiver");
      }
    
    }
    delay(10);
  } else {
      if (radio.available()) {              // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
        radio.read(&payload, bytes);             // fetch payload from FIFO
        if (bytes != PAYLOAD_LENGTH) {
          if (Serial) {
            Serial.println("Invalid payload length");
            continue;
          }
        }
        int speed = payload;
        if (Serial) {
          Serial.print("Got payload");
          Serial.print(" Length ");
          Serial.print(bytes);
          Serial.print(" Payload ");
          Serial.println(payload, DEC);
        }
        all(speed);
      }
      delay(10);
  }
}
