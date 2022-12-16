//Libraries
#include <SoftwareSerial.h>

//Pin numberings
#define RXpin 6
#define TXpin 5
#define LED 2
#define buttonPin 8
#define resistorBreak1 9
#define resistorBreak2 7
#define resistorBreak3 10

//RS485 module definition
#define serialControl 3
#define RS485transmit     HIGH
#define RS485receive     LOW
SoftwareSerial RS485Serial(RXpin, TXpin); //Constructor of the RS485 module

//Global variables
boolean interfaceButton = LOW;

void setup() {
  //Set the resistor pins to output.
  //These pins are used to keep the external power bank on
  pinMode(resistorBreak1, OUTPUT);
  pinMode(resistorBreak2, OUTPUT);
  pinMode(resistorBreak3, OUTPUT);
  digitalWrite(resistorBreak1, HIGH);
  digitalWrite(resistorBreak2, HIGH);
  digitalWrite(resistorBreak3, HIGH);

  //Set pin configuration
  pinMode(LED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(serialControl, OUTPUT);

  //Set serial connections
  Serial.begin(9600);
  RS485Serial.begin(4800);
}

void loop() {
  //Keep checking if interfaceButton has been pressed, until it is pressed
  while (interfaceButton == LOW) {
    interfaceButton = digitalRead(buttonPin);
  }
  //Button was pressed, so let's start a measurement session
  //Turn off LED to indicate that we are trying to send a message
  //to all slaves to start measurements
  digitalWrite(serialControl, RS485transmit);

  //Broadcast to all slaves to start a new measurements
  RS485Serial.println("1");

  //Indicate that broadcast was successful by blinking the LED
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
