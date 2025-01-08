#ifndef RCCAR_H
#define RCCAR_H

#include <wiringPi.h>
#include <wiringPiSPI.h>
#undef INT_EDGE_FALLING
#undef INT_EDGE_RISING
#undef INT_EDGE_BOTH
#undef LOW
#undef HIGH
#undef INPUT
#undef OUTPUT
#undef delay
#undef delayMicroseconds
#include <softPwm.h>
#include <iostream>
#include <cstdlib>
//#include <bcm2835.h>
#include "/usr/local/include/RF24/RF24.h"
#include <mqtt/client.h>
#include <mqtt/message.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace mqtt;

/* Constants */
const int EIGHT_BIT = 255;
const int TEN_BIT = 1023;

/* Data Packet */
struct Packet {
    uint8_t Steering;
    uint8_t Throttle;
    int Lights;
};

/* SPI & RF24 */
extern RF24 radio;
extern const uint8_t address[6];

/* Steering and Throttle */
extern const int STEERING;
extern const int THROTTLE;
float steering_map(float input);

/* Lights */
extern const int HEAD_LIGHTS;
extern const int TAIL_LIGHTS;

/* State Machine Classes */
class State {
public:
    virtual ~State() = default;
    virtual void behaviour() = 0;
    virtual void displayState() = 0;
    virtual State* getNextState() = 0;
};

class Brightest : public State {
public:
    void displayState() override;
    void behaviour() override;
    State* getNextState() override;
};

class Dimmer : public State {
public:
    void displayState() override;
    void behaviour() override;
    State* getNextState() override;
};

class Off : public State {
public:
    void displayState() override;
    void behaviour() override;
    State* getNextState() override;
};

class lightController {
private:
    State* currentState = nullptr;
public:
    void init();
    ~lightController();
    void transition();
};

/* MQTT and Telemetry */
extern string ip;
extern string id;

#endif