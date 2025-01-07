#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <SPI.h>


/* Steering */
const int POT_PIN = 34;

/* Throttle */
const int TRIG_PIN = 35;

/* Lights */
const int POWER_LED = 4;
const int LIGHTS_BUTTON = 0;

/* SPI & RF24 */
const int SPI_CE = 5;
const int SPI_CSN = 18;
RF24 radio(SPI_CE, SPI_CSN);
const byte address[6] = "00001"; // Implicit '/0' terminator for compatibility with string operations

/* Data Packet */
struct Packet{
  byte Steering;
  byte Throttle;
  int Lights; 
};
Packet data;

void setup() {
  /* Power led */
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);

  /*Light button */
  pinMode(LIGHTS_BUTTON, INPUT_PULLUP);

  /* SPI */
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX); // Max power
  radio.stopListening(); // Sets this as a transmitter
}

void loop() {
  /* Steering */
  data.Steering = map(analogRead(POT_PIN), 0, 4095, 0, 255);

  /* Throttle */
  data.Throttle = map(analogRead(TRIG_PIN), 0, 4095, 0, 255);
  
  /* Lights */
  if (digitalRead(LIGHTS_BUTTON) == LOW){
    if (data.Lights == 0){
      data.Lights = 1;
    } else if (data.Lights == 1){
      data.Lights = 0;
    }
  }

  /* SPI & RF24 */
  radio.write(&data, sizeof(Packet));
}
