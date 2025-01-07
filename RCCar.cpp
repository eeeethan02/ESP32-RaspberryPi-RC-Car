class Dimmer;
class Off;

#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <cstdlib>
#include <bcm2835.h>
#include "/usr/local/include/RF24/RF24.h"
#include <mqtt/client.h>
#include <mqtt/message.h>
#include <chrono>
#include <thread>

// Undefine macros that conflict
#undef INT_EDGE_FALLING
#undef INT_EDGE_RISING
#undef INT_EDGE_BOTH
#undef LOW
#undef HIGH
#undef INPUT
#undef OUTPUT
#undef delay
#undef delayMicroseconds

#define OUTPUT 1

using namespace std;
using namespace mqtt;

/* Bits */
const int EIGHT_BIT = 255;
const int TEN_BIT = 1023;

/* Data Packet */
struct Packet{
    uint8_t Steering;
    uint8_t Throttle;
    int Lights;
};
Packet controls;

/* SPI & RF24*/
RF24 radio(22, 8); // May be 0 instead of 8 but probably not
const uint8_t address[6] = "00001";

/* Steering */
const int STEERING = 18; // PWM 0
float steering_map (float input){
    return (float(185/1024) + 75);
}

/* Throttle */
const int THROTTLE = 13;

/* Lights */
const int HEAD_LIGHTS = 23; // Normal GPIO Pins
const int TAIL_LIGHTS = 24;

class State {
public:
    virtual ~State() = default;
    virtual void behaviour() = 0;
    virtual void displayState() = 0;
    virtual State* getNextState() = 0;
};

class Brightest : public State {
public:
    void displayState() override {
        cout << "LED state: Brightest";
    }
    void behaviour() override {
        softPwmWrite(HEAD_LIGHTS, EIGHT_BIT);
        softPwmWrite(TAIL_LIGHTS, EIGHT_BIT);
    }
    State* getNextState() override {

        return new Dimmer();  // This will work now
    }
};

class Dimmer : public State {
public:
    void displayState() override {
        cout << "LED state: Dimmer";
    }
    void behaviour() override {
        softPwmWrite(HEAD_LIGHTS, EIGHT_BIT * 0.5);
        softPwmWrite(TAIL_LIGHTS, EIGHT_BIT * 0.5);
    }
    State* getNextState() override {
        return new Off();  // This will work now
    }
};

class Off : public State {
public:
    void displayState() override {
        cout << "LED state: Off";
    }
    void behaviour() override {
        softPwmWrite(HEAD_LIGHTS, 0);
        softPwmWrite(TAIL_LIGHTS, 0);
    }
    State* getNextState() override {
        return new Brightest();
    }
};

// Maybe implement strobe in future
/*
class Strobe : public State {
    public:
    void displayState() override {
        cout << "LED state: Strobe";
    };
    void behaviour() override {
        pwmSetMode(PWM_MODE_BAL);
        pwmSetClock(4096);
        pwmWrite(HEAD_LIGHTS, EIGHT_BIT*0.5);
        pwmWrite(TAIL_LIGHTS, EIGHT_BIT*0.5);
        }
    State* getNextState() override {
        return new Off();
    }
};
*/

class lightController {
    private:
    State* currentState = nullptr;
    public:
    void init() {
        currentState = new Off();
        currentState->behaviour();
    }
    ~lightController(){
        delete currentState;
    }
    void transition(){
        State* newState = lightController::currentState->getNextState();
        delete currentState;
        currentState = newState;
        currentState->behaviour();
    }

};

/* MQQT and Telemtry*/
    string ip = "localhost:1883";
    string id = "publisher1";

int main(){
    /* Check wiringPi Initialization */
    if (wiringPiSetupGpio() < 0){
        std::cout << "Pigpio initialization failed." << std::endl;
        return EXIT_FAILURE;
    }

    /* SPI and RF24*/
    if (!bcm2835_init()) {
    cout << "bcm2835 initialization failed!" << endl;
    return 1;
    }
    if (!radio.begin()) {
    cout << "RF24 initialization failed!" << endl;
    return 1;
    }
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_1MBPS);
    radio.setChannel(108);
    radio.openWritingPipe(address);
    radio.startListening();
    radio.printDetails();

    /* Steering */
    pinMode(STEERING, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_BAL);
    pwmSetRange(TEN_BIT);
    pwmSetClock(192);
    pwmWrite(STEERING, 0);

    /* Throttle */
    pinMode(THROTTLE, PWM_OUTPUT); // Same mode, clock and range for both PWM channels
    pwmWrite(THROTTLE, 0);
    
    /* Lights */
    pinMode(HEAD_LIGHTS, OUTPUT);
    pinMode(TAIL_LIGHTS, OUTPUT);
    softPwmCreate(HEAD_LIGHTS, 0, EIGHT_BIT);
    softPwmCreate(TAIL_LIGHTS, 0, EIGHT_BIT);
    lightController controller;
    controller.init(); 

    /* MQQT and Telemetry*/
    client client1(ip, id, create_options(MQTTVERSION_5)); // Create client with broker ip, id, mqtt version
    client1.connect();
    message_ptr telemetry = make_message("telemetry", ""); // set default message topic and message
    float speed = 0, batteryVoltage = 0, batteryTemperature = 0;
    float lastTelemetryUpdate = 0;

    while(true) {
        /* SPI and RF24 */
        if (radio.available()){
            radio.read(&controls, sizeof(controls));
        }

        /* Steering */
        pwmWrite(STEERING, steering_map(controls.Steering));

        /* Throttle */
        pwmWrite(THROTTLE, controls.Throttle);

        /* Lights */
        if (controls.Lights == 1){
            controller.transition();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

        /* MQQT and Telemetry */
        if (lastTelemetryUpdate - millis() >= 500){
            telemetry->set_payload("Speed: " + std::to_string(speed) + ", Battery Voltage: " + std::to_string(batteryVoltage) + ", Battery Temperature: " + std::to_string(batteryTemperature));
            client1.publish(telemetry);
            lastTelemetryUpdate = millis();
        }
        
    }
    bcm2835_close();
    client1.disconnect();
}