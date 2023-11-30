// Libraries
#include <AccelStepper.h>

// Define Pins
const int stepPin = 10;    // pin for controlling the stepping via the driver
const int dirPin = 8;     // pin for controlling direction of stepper
const int enPin = 7;      // pin for enabling stepper driver
const int torquePin = A0;  // pin for reading the voltage proportional to the torque experience by the machine

// Define variables for RPM and microstepping
float rpm_float = 0;
float micro_steps_float = 0;
String on_off = "0";
float interval = 500;
float angle = 0;
float stepperSpeed;
float ratio = 6;

// Manual control variables
float step_time = 500;
String manual_direction = "off";

// Define variables for fatigue test
float max_torque = 0;
float fatigue_cycles = 0;

// Define test case string
String test_case = "standard";  // Test case, standard by default

// Initialise Timing
unsigned long previousMillis = 0;

// Displacement
float displacement = 0;

// Define Stepper
AccelStepper stepper(1, stepPin, dirPin);

// Message Types
String MSG_TYPE_SET_CONFIG = "SET_CFG";  // sets settings for tests
String MSG_TYPE_SET_TEST = "SET_TEST";   // dictates start/stop of tests and test type
String MSG_TYPE_SET_MANUAL = "SET_MC";   // controls manual CW and CCW turning of the stepper

void setup() {
  stepper.setMaxSpeed(1000);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW); // enables the stepper driver

  Serial.begin(115200);  // Initialize serial communication with a baud rate of 115200.
}

void readSerialData() {
  static String inputData;
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n' || incomingChar == '\r') {
      processData(inputData);
      inputData = "";  // Clear the string for the next line
    } else {           // Ignore carriage return
      inputData += incomingChar;
    }
  }
}

void processData(String data) {
  // Accepts strings of the form: msgType; Parameter_1, Value_1; Parameter_2, Value_2; Parameter_n, Value_n;
  int firstCommaIndex = data.indexOf(';');
  String msgType = data.substring(0, firstCommaIndex);
  String keyValuePairs = data.substring(firstCommaIndex + 1);

  if (msgType == MSG_TYPE_SET_CONFIG) {
    handleStepperConfig(msgType, keyValuePairs);
  } else if (msgType == MSG_TYPE_SET_TEST) {
    handleTest(msgType, keyValuePairs);
  } else if (msgType == MSG_TYPE_SET_MANUAL) {
    handleManualControl(msgType, keyValuePairs);
  } else {
    Serial.println("Unknown Message Type");
  }
}

void handleStepperConfig(String msgType, String keyValuePairs) {
  int semicolonIndex = 0;
  while ((semicolonIndex = keyValuePairs.indexOf(';')) != -1) {
    String pair = keyValuePairs.substring(0, semicolonIndex);
    int commaIndex = pair.indexOf(',');

    if (commaIndex != -1) {
      String key = pair.substring(0, commaIndex);
      String value = pair.substring(commaIndex + 1);

      // Using abbreviations for keys
      if (key == "MS") {  // MS = microSteps
        micro_steps_float = value.toFloat();
        Serial.println("Micro steps: " + value);
      } else if (key == "CS") {  // CS = setSpeed
        rpm_float = value.toFloat();
        float steps_rev = micro_steps_float * ratio;
        angle = (360 / (micro_steps_float * ratio));
        stepperSpeed = steps_rev * rpm_float / 60;
        stepper.setSpeed(stepperSpeed);
        Serial.println("Stepper speed: " + value);
      } else if (key == "MT") {  // MT = maxTorque
        max_torque = value.toFloat();
        Serial.println("Max torque: " + value);
      } else if (key == "FC") {  // FC = fatigueCycles}
        fatigue_cycles = value.toFloat();
        Serial.println("Fatigue cycles: " + value);
      } else {
        Serial.println("Unknown command: " + key);
      }
    } else {
      Serial.println("Invalid pair format: " + pair);
    }

    // Update keyValuePairs for the next iteration
    keyValuePairs = keyValuePairs.substring(semicolonIndex + 1);
  }
}

void handleTest(String msgType, String keyValuePairs) {
  int semicolonIndex = 0;
  while ((semicolonIndex = keyValuePairs.indexOf(';')) != -1) {
    String pair = keyValuePairs.substring(0, semicolonIndex);
    int commaIndex = pair.indexOf(',');

    if (commaIndex != -1) {
      String key = pair.substring(0, commaIndex);
      String value = pair.substring(commaIndex + 1);

      // Using abbreviations for keys
      if (key == "OF") {  // OF = on_off
        on_off = value;
      } else if (key == "TC") {  // TC = testCase
        test_case = value;
      } else {
        Serial.println("Unknown command: " + key);
      }
    } else {
      Serial.println("Invalid pair format: " + pair);
    }

    // Update keyValuePairs for the next iteration
    keyValuePairs = keyValuePairs.substring(semicolonIndex + 1);
  }
  stepper.setSpeed(stepperSpeed);
}

void handleManualControl(String msgType, String keyValuePairs) {
  int semicolonIndex = 0;
  while ((semicolonIndex = keyValuePairs.indexOf(';')) != -1) {
    String pair = keyValuePairs.substring(0, semicolonIndex);
    int commaIndex = pair.indexOf(',');

    if (commaIndex != -1) {
      String key = pair.substring(0, commaIndex);
      String value = pair.substring(commaIndex + 1);

      // Using abbreviations for keys
      if (key == "DIR") {  // DIR = direction
        manual_direction = value;
      } else if (key == "ST") {  // ST = stepTime
        step_time = value.toFloat();
      } else {
        Serial.println("Unknown command: " + key);
      }
    } else {
      Serial.println("Invalid pair format: " + pair);
    }

    // Update keyValuePairs for the next iteration
    keyValuePairs = keyValuePairs.substring(semicolonIndex + 1);
  }
  if (manual_direction == "CW") {
    stepper.setSpeed(stepperSpeed);
  } else {
    stepper.setSpeed(-stepperSpeed);
  }
  previousMillis = millis();
}

void standardTest() {
  // Steps motor at desired speed
  stepper.runSpeed();
}

void fatigueTest() {
  // Add code for fatigue test here
}

void loop() {
  readSerialData();

  if (manual_direction == "CW" || manual_direction == "CCW") {
    unsigned long currentMillis = millis();
    stepper.runSpeed();
    if (currentMillis - previousMillis >= step_time) {
      previousMillis = currentMillis;
      manual_direction = "off";
    }
  }
  if (on_off == "1") {
    // Time Counting
    unsigned long currentMillis = millis();

    if (test_case == "standard") {
      standardTest();
    } else if (test_case == "fatigue") {
      fatigueTest();
    } else {
      Serial.println("Unknown test type: " + test_case);
    }

    if (currentMillis - previousMillis >= 500) {
      // Recording Position
      displacement += angle * 0.5 * stepperSpeed;
      previousMillis = currentMillis;
      Serial.print(displacement);
      Serial.print(",");
      Serial.print(7);  // Placeholder value for torque, will be changed to a value proportional to the reading from A0
      Serial.print("\n");
    }
  } else if (on_off == "0") {
    displacement = 0;
  } else {
    Serial.println("Error runnings test");
  }
}
