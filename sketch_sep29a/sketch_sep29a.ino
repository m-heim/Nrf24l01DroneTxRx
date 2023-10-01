#include <RF24_config.h>
#include <printf.h>
#include <RF24.h>
#include <nRF24L01.h>

#define MOTOR_1 3
#define MOTOR_2 5
#define MOTOR_3 6
#define MOTOR_4 9
#define SENDER 0
#define SERIAL 1
#define X_LEFT A4
#if SENDER
#define CE_PIN 9
#define CSN_PIN 10
#else 
#define CE_PIN 7
#define CSN_PIN 8
#endif

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
int radioNumber = SENDER;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
char payload = 0;

void all(int speed) {
  analogWrite(MOTOR_1, speed);
  analogWrite(MOTOR_2, speed);
  analogWrite(MOTOR_3, speed);
  analogWrite(MOTOR_4, speed);
}

void setup() {
  if (SERIAL) {
    Serial.begin(115200);
    while (!Serial) {
    }
    Serial.println("Serial initialized");
  }
    if (!radio.begin()) {
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      if (SERIAL) {
        Serial.println("Device is not responding");
      }
      digitalWrite(LED_BUILTIN, HIGH);
    }  // hold in infinite loop
  } else {
    if (SERIAL) {
      Serial.println("Nrf radio successfully connected");
    }
  }
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(payload));
  if (!SENDER) {
    pinMode(10, OUTPUT);
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
  if (SERIAL) {
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
    val = val / 2;
    payload = (char) val;
    bool report = radio.write(&payload, sizeof(payload));
    if (SERIAL) {
      Serial.println(raw, DEC);
      Serial.println(val, DEC);
      if (report) {
        Serial.println("Message was successfully transmitted");
      } else {
        Serial.println("No ack from receiver"); 
      }
      Serial.println(radio.isChipConnected());
    
    }
    delay(1000);
  } else {
      if (radio.available()) {              // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
        radio.read(&payload, bytes);             // fetch payload from FIFO
        int speed = payload;
        if (SERIAL) {
          Serial.print("Got payload");
          Serial.print(" Length ");
          Serial.print(bytes);
          Serial.print(" Payload ");
          Serial.println(payload, DEC);
        }
        all(speed);
      } else {
        if (SERIAL) {
          Serial.println("No message received");
        }
      }
      delay(100);
  }
}
