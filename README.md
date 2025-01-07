An RC car controlled via a Raspberry Pi and ESP32 microcontroller. The Raspberry Pi handles telemetry, camera feed, and motor control, while the ESP32 manages the wireless communication for control signals.
The car features PWM-controlled steering and throttle, as well as customizable LED light states (Brightest, Dimmer, Off).

Components Used:
  Raspberry Pi
  ESP32 Microcontroller
  RF24 Module for wireless communication
  Motor Driver (DRV8871 or similar)
  PWM-controlled Steering and Throttle Motors
  LEDs (Headlights and Tail Lights)
Prerequisites
  Before running the code, ensure that the following components and software are installed on your Raspberry Pi and ESP32:

Raspberry Pi Setup
  Operating System: Raspbian (latest version)
Libraries:
  WiringPi: Used for controlling the GPIO pins for PWM signals.
  RF24: For SPI communication with the RF24 module.
  MQTT: For telemetry and control communication.
  bcm2835 or WiringPi for GPIO control.
Hardware:
  RF24 module connected to the Raspberry Pi via SPI pins (SCK, MOSI, MISO, and CE).
  Motor driver (e.g., DRV8871) connected to the Raspberry Pi GPIO pins for controlling the throttle and steering motors.
  LED lights (Headlights and Tail Lights) connected to GPIO pins.
  ESP32 Setup
  ESP32 Dev Board (e.g., ESP32 DevKitC)
  Arduino IDE (or PlatformIO with ESP32 toolchain)
  Install the ESP32 board via the Board Manager in the Arduino IDE.
  Libraries for ESP32 (Install via Arduino IDE Library Manager):
  nRF24 (for communication with the Raspberry Pi via RF24)
  Wire (for I2C communication if needed)
