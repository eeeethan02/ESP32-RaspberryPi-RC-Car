#include "RCCar.h"

int main(){
    /* Check wiringPi Initialization */
    if (wiringPiSetupGpio() < 0){
        std::cout << "WiringPi initialization failed." << std::endl;
        return EXIT_FAILURE;
    }

    /* SPI and RF24*/
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
    Packet controls;

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
        if (millis() - lastTelemetryUpdate >= 500){
            telemetry->set_payload("Speed: " + std::to_string(speed) + ", Battery Voltage: " + std::to_string(batteryVoltage) + ", Battery Temperature: " + std::to_string(batteryTemperature));
            client1.publish(telemetry);
            lastTelemetryUpdate = millis();
        }
        
    }
    client1.disconnect();
}