#include <RF24.h>
#include <nRF24L01.h>
#include <Servo.h>

#define MOTOR 3
#define FORWARD 4
#define REVERSE 5
#define SERVO 6
#define SENDER 0
#define X_LEFT A4
#define Y_RIGHT A7
#define CE_PIN 9
#define CSN_PIN 10

#define PAYLOAD_LENGTH 4

RF24 radio(CE_PIN, CSN_PIN);
Servo servo;
uint8_t address[][6] = { "1Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
int radioNumber = SENDER;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
int8_t payload[PAYLOAD_LENGTH];

void drive(int speed) {
  if (speed > 0) {
    digitalWrite(FORWARD, HIGH);
    digitalWrite(REVERSE, LOW);
    analogWrite(MOTOR, speed);
  } else if (speed == 0) {
    digitalWrite(FORWARD, LOW);
    digitalWrite(REVERSE, LOW);
    analogWrite(MOTOR, speed);
  } else {
    digitalWrite(FORWARD, LOW);
    digitalWrite(REVERSE, HIGH);
    analogWrite(MOTOR, -speed);
  }
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
    pinMode(SERVO, OUTPUT);
    pinMode(MOTOR, OUTPUT);
    servo.attach(SERVO);
    radio.openReadingPipe(1, address[0]);
    radio.startListening();
  } else {
    pinMode(X_LEFT, INPUT);
    pinMode(Y_RIGHT, INPUT);
    radio.openWritingPipe(address[0]);
    radio.stopListening();
  }
  if (Serial) {
    Serial.println("Device set up successfully");
  }
}

void loop() {
  if (SENDER) {
    int16_t rawx = analogRead(X_LEFT);
    int16_t valx = rawx - 505;
    int16_t rawy = analogRead(Y_RIGHT);
    int16_t valy = rawy - 501;
    valx = -valx;
    valx = valx / 4;
    valx = map(valx, -150, 150, -120, 120);
    valy = -valy;
    valy = valy / 4;
    valy = map(valy, -150, 150, -120, 120);
    payload[0] = valx;
    payload[1] = valy;
    bool report = radio.write(&payload, sizeof(payload));
    if (Serial) {
      Serial.println(valx, DEC);
      Serial.println(valy, DEC);
      if (report) {
        Serial.println("Message was successfully transmitted");
      } else {
        Serial.println("No ack from receiver");
      }
    
    }
    delay(100);
  } else {
      if (radio.available()) {              // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
        radio.read(&payload, bytes);             // fetch payload from FIFO
        if (bytes != PAYLOAD_LENGTH) {
          if (Serial) {
            Serial.println("Invalid payload length");
            return;
          }
        }
        int speed = payload[0];
        int steering = payload[1];
        int mappedSteering = map(steering, -120, 120, 0, 180);
        int mappedSpeed = map(speed, -120, 120, -250, 250);
        drive(mappedSpeed);
        servo.write(mappedSteering);
        if (Serial) {
          Serial.print("Got payload");
          Serial.print(" Length ");
          Serial.print(bytes);
          Serial.print(" Payload ");
          Serial.print(payload[0]);
          Serial.print(payload[1]);
        }
        
      }
      delay(100);
  }
}
