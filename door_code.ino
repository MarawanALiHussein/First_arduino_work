
#include <Keypad.h>
#include <Servo.h>

Servo lockServo;

const int SERVO_PIN = 10;
const String PASSWORD = "1234";

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, A1};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

String enteredPassword = "";

const int LOCKED_ANGLE = 0;
const int UNLOCKED_ANGLE = 90;

void setup() {
  Serial.begin(9600);

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCKED_ANGLE);

  Serial.println("Enter password, then press #");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key >= '0' && key <= '9') {
      enteredPassword += key;

      // Display the actual digits
      Serial.println(enteredPassword);
    }

    else if (key == '*') {
      enteredPassword = "";
      Serial.println("Password cleared");
    }

    else if (key == '#') {
      Serial.println("Checking password...");

      if (enteredPassword == PASSWORD) {
        Serial.println("Access Granted!");

        lockServo.write(UNLOCKED_ANGLE);
        delay(5000);

        lockServo.write(LOCKED_ANGLE);
        Serial.println("Door Locked Again");
      }
      else {
        Serial.println("Wrong Password!");
      }

      enteredPassword = "";
      Serial.println("Enter password, then press #");
    }
  }
}
