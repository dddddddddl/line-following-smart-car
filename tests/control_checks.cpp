// Runs the actual sketch's control functions with minimal hardware stubs.
// No Arduino board, motors, LEDs or sensor bus is accessed by these checks.
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
using std::min;
using std::max;

static unsigned long hostTime = 0;
unsigned long millis() { return hostTime; }
struct SerialStub {
    void begin(int) {}
    template<class T> void print(T) {}
    template<class T> void println(T) {}
};
SerialStub Serial;

// Skip the vendored header only for this host harness and supply its tiny UI API.
// The separate AVR build compiles the original NeoPixel implementation.
#define ADAFRUIT_NEOPIXEL_H
#define NEO_GRB 0
#define NEO_KHZ800 0
class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(int, int, int) {}
    void begin() {}
    void show() {}
    void setPixelColor(int, uint32_t) {}
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return (uint32_t(r) << 16) | (uint32_t(g) << 8) | b;
    }
};

// Arduino sketch preprocessing normally generates these declarations.
void updateState();
void controlMotors();
void updateLEDs();
void debugOutput();
int smoothSpeed(int, int);
#include "../firmware/line_following_car/line_following_car.ino"

SENSORS sensor{};
static int observedLeft = 0, observedRight = 0;
void shift_reg_init() {}
void reload_shift_reg() {}
void motor_init() {}
void motor_set_PWM(int left, int right) {
    observedLeft = left;
    observedRight = right;
}

void resetController() {
    sensor = {};
    currentState = FOLLOWING_LINE;
    lastLineDetectedTime = 0;
    lastLeftSpeed = lastRightSpeed = 0;
    hostTime = 0;
}
void settleMotorCommand() {
    for (int i = 0; i < 30; ++i) controlMotors();
}

int main() {
    resetController();
    sensor.ir_mid = 1;
    hostTime = 100;
    updateState();
    settleMotorCommand();
    assert(currentState == FOLLOWING_LINE);
    assert(observedLeft == 150 && observedRight == 150);
    std::cout << "PASS: centered line commands equal forward PWM\n";

    resetController();
    sensor.ir_left_3 = 1;
    settleMotorCommand();
    assert(observedLeft == -255 && observedRight == 255);
    resetController();
    sensor.ir_right_3 = 1;
    settleMotorCommand();
    assert(observedLeft == 255 && observedRight == -255);
    std::cout << "PASS: outer sensors request the supplied sharp-turn commands\n";

    resetController();
    sensor.ir_left_1 = 1;
    settleMotorCommand();
    assert(observedLeft == 75 && observedRight == 200);
    resetController();
    sensor.ir_right_2 = 1;
    settleMotorCommand();
    assert(observedLeft == 200 && observedRight == 50);
    std::cout << "PASS: inner and middle sensors select distinct turn commands\n";

    resetController();
    hostTime = 500;
    updateState();
    assert(currentState == FOLLOWING_LINE);
    hostTime = 501;
    updateState();
    assert(currentState == SEARCHING_LINE);
    settleMotorCommand();
    assert(observedLeft == 120 && observedRight == -120);
    // This is an observed limitation, not a newly implemented search timeout.
    hostTime = 100000;
    updateState();
    assert(currentState == SEARCHING_LINE);
    std::cout << "PASS: loss beyond 500 ms starts search; search has no automatic LOST transition\n";

    sensor.ir_mid = 1;
    updateState();
    assert(currentState == FOLLOWING_LINE);
    assert(lastLineDetectedTime == hostTime);
    std::cout << "PASS: reacquisition restores following\n";

    assert(smoothSpeed(0, 150) == 20);
    assert(smoothSpeed(150, 0) == 130);
    assert(smoothSpeed(-10, 0) == 0);
    assert(smoothSpeed(10, -255) == -10);
    std::cout << "PASS: per-iteration PWM ramp is bounded and does not overshoot targets\n";
}
