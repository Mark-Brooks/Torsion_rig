#include <AccelStepper.h>

#define str(s) #s
// message types
#define MSG_TYPE_SET_STEPPER str(SET_STPR)
#define MSG_TYPE_SET_MEASUREMENT str(SET_MEAS)   // configure measurement, speed, start stop etc?
#define MSG_TYPE_SET_MEASUREMENT str(INFO_MEAS)  // for raw measurements

// Initialise Timings
#define stepPin 5
#define dirPin 2
#define enPin 8
#define ratio 6

// Create an AccelStepper object
AccelStepper stepper(1, stepPin, dirPin);

void setup() {
  // Start serial communication at 9600 baud rate
  Serial.begin(115200);  // mark you should use at least 115200, 9600 is a really old legacy speed
  Serial.println("Mark's Stepper Controller");

  // Other setup code for the stepper or other components
}

// probs need some forward declarations so the compiler is happy
//when this gets big it makes sense to split this out into their own classes or least
//distinct header files
void readSerialData();
void processData(String data);
void handleMessageType(int msgType, String keyValuePairs);
void handleStepperConfig(String keyValuePairs);
void handleMeasurementConfig(String keyValuePairs);
void sendStepperSettings();


void sendConfigToLabview(String config) {
  // Abbreviations: CS = Current Speed, MS = Max Speed, AC = Acceleration, CP = Current Position
  String settings = "";
  if (config == MSG_TYPE_SET_STEPPER) {
    settings = config + ',' + "CS," + String(stepper.speed()) + ";MS," + String(stepper.maxSpeed()) + ";AC," + String(stepper.acceleration()) + ";CP," + String(stepper.currentPosition()) + "\n";
  } else if (config == MSG_TYPE_SET_STEPPER) {
    // handle other cases
  }
  Serial.println("sendConfigToLabview() -> ");
  Serial.println(settings);
}

// Function to read serial data
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

// Function to process incoming data
void processData(String data) {
  int firstCommaIndex = data.indexOf(';');
  String msgType = data.substring(0, firstCommaIndex);
  String keyValuePairs = data.substring(firstCommaIndex + 1);

  Serial.println("processData() -> ");
  String debug = "";
  debug += "msgType: ";
  debug += msgType;
  debug += " keyValuePairs: ";
  debug += keyValuePairs;
  Serial.println(debug);

  if (msgType == MSG_TYPE_SET_STEPPER) {
    handleStepperConfig(msgType, keyValuePairs);
  } else if (msgType == MSG_TYPE_SET_MEASUREMENT) {
    handleMeasurementConfig(msgType, keyValuePairs);
  } else {
    Serial.println("Unknown Message Type");
  }
}

// do the config update
void handleStepperConfig(String msgType, String keyValuePairs) {
  int semicolonIndex = 0;
  Serial.println("handleStepperConfig() -> " + String(semicolonIndex));
  while ((semicolonIndex = keyValuePairs.indexOf(';')) != -1) {
    String pair = keyValuePairs.substring(0, semicolonIndex);
    int commaIndex = pair.indexOf(',');

    if (commaIndex != -1) {
      String key = pair.substring(0, commaIndex);
      String value = pair.substring(commaIndex + 1);
      Serial.print(key + " -> ");

      // Using abbreviations for keys
      if (key == "CS") {  // CS = setSpeed
        stepper.setSpeed(value.toFloat());
        Serial.println("setSpeed() -> " + value);
      } else if (key == "MS") {  // MS = setMaxSpeed
        stepper.setMaxSpeed(value.toFloat());
        Serial.println("setMaxSpeed() -> " + value);
      } else if (key == "AC") {  // AC = setAcceleration
        stepper.setAcceleration(value.toFloat());
        Serial.println("setAcceleration() -> " + value);
      } else if (key == "MT") {  // MT = moveTo
        stepper.moveTo(value.toInt());
        Serial.println("moveTo() -> " + value);
      } else {
        Serial.println("Unknown command: " + key);
      }
    } else {
      Serial.println("Invalid pair format: " + pair);
    }

    // Update keyValuePairs for the next iteration
    keyValuePairs = keyValuePairs.substring(semicolonIndex + 1);
  }
  sendConfigToLabview(msgType);  // After setting configurations, send the new settings back
}

// Placeholder function to handle measurement configuration
void handleMeasurementConfig(String msgType, String keyValuePairs) {
  // Implement the logic to handle measurement configuration
  // based on the keyValuePairs
  sendConfigToLabview(msgType);  // After setting configurations, send the new settings back
}

void loop() {
  readSerialData();
  // Other loop code can go here
}

// Test Cases Serial Data

// 1. Test Stepper Configuration Handling
// Send: "SET_STPR;CS,100;MS,200;AC,50,MT,123\n"
