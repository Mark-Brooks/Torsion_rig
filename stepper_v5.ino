// Libraries
#include <AccelStepper.h>

// Initialise Timings
const int stepPin = 5;
const int dirPin = 2;
const int enPin = 8;
const float ratio = 6;

// Define variables for RPM and microstepping
float rpm_float = 0;
float micro_steps_float = 0;
int stepState = LOW;
String on_off = "0";
float interval = 500;
float angle = 0;
float stepperSpeed;

// Define initial timings
unsigned long previousMillis = 0;

// Displacement
float displacement = 0;

// Defined Stepper
AccelStepper stepper(1, stepPin, dirPin);

void setup() {
  stepper.setMaxSpeed(1000);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);

  Serial.begin(115200);  // Initialize serial communication with a baud rate of 9600.
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\r');

    // Find the positions of commas in the input string
    int comma1 = command.indexOf(',');
    int comma2 = command.lastIndexOf(',');

    if (comma1 != -1 && comma2 != -1) {
      // Extract and store the values between commas
      on_off = command.substring(0, comma1);
      String micro_steps = command.substring(comma1 + 1, comma2);
      String rpm = command.substring(comma2 + 1);

      // Convert the extracted strings to integers
      micro_steps_float = micro_steps.toFloat();
      rpm_float = rpm.toFloat();

      // Calculate steps per revolution given gear ratio and angle per step
      float steps_rev = micro_steps_float * ratio;
      angle = (360 / (micro_steps_float * ratio));
      stepperSpeed = steps_rev * rpm_float / 60;

      // Stepper Speed
      stepper.setSpeed(stepperSpeed);
    }
  }
  if (on_off == "0") {
    displacement = 0;
  }
  if (on_off == "1") {
    // Time Counting
    unsigned long currentMillis = millis();

    // Step Motor
    stepper.runSpeed();

    if (currentMillis - previousMillis >= 500) {
      // Recording Position
      displacement += angle * 0.5 * stepperSpeed;
      previousMillis = currentMillis;
      Serial.print(displacement);
      Serial.print(",");
      Serial.print(7);
      Serial.print("\n");
    }
  }
}
