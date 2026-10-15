#include "Adafruit_NeoPixel.h"
#include "comm.h"
#include "motor.h"

#define PIN 4
#define NUMPIXELS 2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Motor parameters
#define BASE_SPEED 150
#define TURN_SPEED 200
#define SHARP_TURN_SPEED 255
#define SEARCH_SPEED 120

// State definitions
enum State {
  FOLLOWING_LINE,
  SEARCHING_LINE,
  LOST
};

State currentState = FOLLOWING_LINE;
unsigned long lastLineDetectedTime = 0;
const unsigned long LOST_TIMEOUT = 500; // Consider the line lost if not detected for 500ms

// Variables for smooth turning
int lastLeftSpeed = 0;
int lastRightSpeed = 0;
const int ACCELERATION = 20; // Maximum speed change per loop iteration

/**
 * Initializes serial communication, shift register, motors, and NeoPixel LEDs.
 */
void setup() {
  Serial.begin(9600);
  shift_reg_init();
  motor_init();
  pixels.begin();
  pixels.show();
}

/**
 * The main loop of the program.
 * It continuously reloads the shift register, updates the state machine, controls the motors,
 * updates the LED display, and outputs debug information.
 */
void loop() {
  reload_shift_reg();

  // 1. Update the state machine
  updateState();

  // 2. Control the motors based on the current state
  controlMotors();

  // 3. Update the LED display
  updateLEDs();

  // 4. Output debug information
  debugOutput();
}

/**
 * Updates the current state of the line-following system.
 * Monitors the state of the line sensors and changes the state if necessary.
 */
void updateState() {
  static bool lastLineState = false;
  bool currentLineState = sensor.ir_mid || sensor.ir_left_1 || sensor.ir_left_2 ||
                         sensor.ir_left_3 || sensor.ir_right_1 || sensor.ir_right_2 ||
                         sensor.ir_right_3;

  if (currentLineState) {
    // If the line is detected, update the time of the last detection and set the state to FOLLOWING_LINE
    lastLineDetectedTime = millis();
    currentState = FOLLOWING_LINE;
  }
  else if (currentState == FOLLOWING_LINE && millis() - lastLineDetectedTime > LOST_TIMEOUT) {
    // If the line was previously being followed but not detected for longer than the timeout,
    // change the state to SEARCHING_LINE
    currentState = SEARCHING_LINE;
  }

  lastLineState = currentLineState;
}

/**
 * Controls the motors based on the current state of the system.
 * Calculates the target speeds for the left and right motors and smoothly adjusts the speeds.
 */
void controlMotors() {
  int targetLeftSpeed = 0;
  int targetRightSpeed = 0;

  switch (currentState) {
    case FOLLOWING_LINE:
      // Normal line-following logic
      if (sensor.ir_mid) {
        // Move straight forward
        targetLeftSpeed = BASE_SPEED;
        targetRightSpeed = BASE_SPEED;
      }
      else if (sensor.ir_left_1 || sensor.ir_left_2 || sensor.ir_left_3) {
        // Turn left
        if (sensor.ir_left_3) {
          // Sharp left turn
          targetLeftSpeed = -SHARP_TURN_SPEED;
          targetRightSpeed = SHARP_TURN_SPEED;
        }
        else if (sensor.ir_left_2) {
          // Moderate left turn
          targetLeftSpeed = BASE_SPEED / 3;
          targetRightSpeed = TURN_SPEED;
        }
        else {
          // Slight left turn
          targetLeftSpeed = BASE_SPEED / 2;
          targetRightSpeed = TURN_SPEED;
        }
      }
      else if (sensor.ir_right_1 || sensor.ir_right_2 || sensor.ir_right_3) {
        // Turn right
        if (sensor.ir_right_3) {
          // Sharp right turn
          targetLeftSpeed = SHARP_TURN_SPEED;
          targetRightSpeed = -SHARP_TURN_SPEED;
        }
        else if (sensor.ir_right_2) {
          // Moderate right turn
          targetLeftSpeed = TURN_SPEED;
          targetRightSpeed = BASE_SPEED / 3;
        }
        else {
          // Slight right turn
          targetLeftSpeed = TURN_SPEED;
          targetRightSpeed = BASE_SPEED / 2;
        }
      }
      break;

    case SEARCHING_LINE:
      // Search mode - Rotate clockwise
      targetLeftSpeed = SEARCH_SPEED;
      targetRightSpeed = -SEARCH_SPEED;
      break;

    case LOST:
      // Completely lost - Stop the motors
      targetLeftSpeed = 0;
      targetRightSpeed = 0;
      break;
  }

  // Smoothly change the speed
  lastLeftSpeed = smoothSpeed(lastLeftSpeed, targetLeftSpeed);
  lastRightSpeed = smoothSpeed(lastRightSpeed, targetRightSpeed);

  motor_set_PWM(lastLeftSpeed, lastRightSpeed);
}

/**
 * Smoothly adjusts the speed from the current value to the target value.
 * Limits the speed change per iteration to ACCELERATION.
 * @param current - The current speed.
 * @param target - The target speed.
 * @return The adjusted speed.
 */
int smoothSpeed(int current, int target) {
  if (current < target) {
    return min(current + ACCELERATION, target);
  }
  else if (current > target) {
    return max(current - ACCELERATION, target);
  }
  return target;
}

/**
 * Updates the NeoPixel LED display based on the current state of the system
 * and the readings from the line sensors.
 */
void updateLEDs() {
  uint8_t r0 = 0, g0 = 0, b0 = 0;
  uint8_t r1 = 0, g1 = 0, b1 = 0;

  // Set base colors according to the state
  if (currentState == FOLLOWING_LINE) {
    // Normal line-following mode
    if (sensor.ir_left_3) g0 = 100;    // Leftmost sensor - green
    if (sensor.ir_left_2) r0 = 100;    // Left middle sensor - red
    if (sensor.ir_left_1) b0 = 100;    // Left inner sensor - blue
    if (sensor.ir_right_1) b1 = 100;   // Right inner sensor - blue
    if (sensor.ir_right_2) r1 = 100;   // Right middle sensor - red
    if (sensor.ir_right_3) g1 = 100;   // Rightmost sensor - green
    if (sensor.ir_mid) r0 = g0 = b0 = r1 = g1 = b1 = 100; // Middle sensor - white
  }
  else if (currentState == SEARCHING_LINE) {
    // Search mode - Blinking yellow
    uint8_t blink = (millis() / 200) % 2 ? 100 : 0;
    r0 = g0 = blink;
    r1 = g1 = blink;
  }
  else {
    // Lost state - Red
    r0 = r1 = 100;
  }

  pixels.setPixelColor(0, pixels.Color(r0, g0, b0));
  pixels.setPixelColor(1, pixels.Color(r1, g1, b1));
  pixels.show();
}

/**
 * Outputs debug information to the serial monitor at regular intervals.
 * Displays the current state, sensor readings, and motor speeds.
 */
void debugOutput() {
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 100) { // Output every 100ms
    Serial.print("State: ");
    switch (currentState) {
      case FOLLOWING_LINE: Serial.print("FOLLOWING"); break;
      case SEARCHING_LINE: Serial.print("SEARCHING"); break;
      case LOST: Serial.print("LOST"); break;
    }

    Serial.print(" | Sensors: ");
    Serial.print(sensor.ir_left_3);
    Serial.print(sensor.ir_left_2);
    Serial.print(sensor.ir_left_1);
    Serial.print(sensor.ir_mid);
    Serial.print(sensor.ir_right_1);
    Serial.print(sensor.ir_right_2);
    Serial.print(sensor.ir_right_3);

    Serial.print(" | Motors: ");
    Serial.print(lastLeftSpeed);
    Serial.print(",");
    Serial.println(lastRightSpeed);

    lastDebugTime = millis();
  }
}
