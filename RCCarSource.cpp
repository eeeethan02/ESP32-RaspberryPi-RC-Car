#include "RCCar.h"

/* SPI and RF24 */
RF24 radio(22, 8);
const uint8_t address[6] = "00001";

/* Steering */
const int STEERING = 18;
float steering_map(float input) {
    return (float(185 / 1024) + 75);
}

/* Throttle */
const int THROTTLE = 13;

/* Lights */
const int HEAD_LIGHTS = 23;
const int TAIL_LIGHTS = 24;

void Brightest::displayState() {
    cout << "LED state: Brightest";
}
void Brightest::behaviour() {
    softPwmWrite(HEAD_LIGHTS, EIGHT_BIT);
    softPwmWrite(TAIL_LIGHTS, EIGHT_BIT);
}
State* Brightest::getNextState() {
    return new Dimmer();
}

void Dimmer::displayState() {
    cout << "LED state: Dimmer";
}
void Dimmer::behaviour() {
    softPwmWrite(HEAD_LIGHTS, EIGHT_BIT * 0.5);
    softPwmWrite(TAIL_LIGHTS, EIGHT_BIT * 0.5);
}
State* Dimmer::getNextState() {
    return new Off();
}

void Off::displayState() {
    cout << "LED state: Off";
}
void Off::behaviour() {
    softPwmWrite(HEAD_LIGHTS, 0);
    softPwmWrite(TAIL_LIGHTS, 0);
}
State* Off::getNextState() {
    return new Brightest();
}

void lightController::init() {
    currentState = new Off();
    currentState->behaviour();
}
lightController::~lightController() {
    delete currentState;
}
void lightController::transition() {
    State* newState = currentState->getNextState();
    delete currentState;
    currentState = newState;
    currentState->behaviour();
}

/* MQTT and Telemetry */
string ip = "localhost:1883";
string id = "publisher1";