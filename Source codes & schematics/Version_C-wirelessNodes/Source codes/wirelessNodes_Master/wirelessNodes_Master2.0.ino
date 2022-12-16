//Libraries
//See documentation at https://nRF24.github.io/RF24
#include <RF24.h>

//Definition of radio ID in the mesh
#define radioID 0

//A struct variable to hold data that will be transmitted through the radio
//It is essentially the payload that will be always transmitted
struct structPayloadRF {
  //This variable will indicate for slaves to perform measurement cycle 1
  boolean buttonToActivateCycle1_CurrentStatus = false;
  //This variable will indicate for slaves to perform measurement cycle 2
  boolean buttonToActivateCycle2_CurrentStatus = false;
};
//Definition of the type payloadRF as a struct of the type structPayloadRF
typedef struct structPayloadRF payloadRF;
//Creation of a payload variable to send data
payloadRF sentPayload;

//Variables to hold state of the transmissions
boolean payloadTransmissionStatus = true;
boolean sendPayloadToSlaves = false;

//Constructor of the RF24 module
RF24 radio(7, 8);

//Array to hold addresses of the nodes in the mesh, so to identify to which we are communicating
//As RF24 documentation says, "it is very helpful to think of an address as a path
//instead of as an identifying device destination"
//In our case, Master will communicate to all nodes at the same time, so there will be
//only one path to communicate to all, and all slaves will hear it.
byte address[][6] = {"1node"};

//Button state variables, to indicate previous and current state
boolean buttonToSelectCycle_CurrentStatus    = HIGH;
boolean buttonToSendPayload_CurrentStatus    = HIGH;
boolean firstLoop = true;

//Define LED and button pins
#define greenLED A0
#define redLED A1
#define pushButtonToSelectCycle 4
#define pushButtonToSendPayload 3
byte resistorNoBreak[4] = {5,6,9,10}; //Resistor to have a minimum current draw avoid powerbank to shutdown

void setup() {
  //Set no-break resistors to provide a minimum current draw so powerbank doesn't turn off
  for (byte i=0;i<=4;i++){
      pinMode(resistorNoBreak[i], OUTPUT);
      digitalWrite(resistorNoBreak[i], HIGH);
  }
  //Initialize RF24 module
  radio.begin();
  //Configure RF24 module to write on the pipe indicated on address[0]
  radio.openWritingPipe(address[0]);
  //Configuring the pins
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(pushButtonToSelectCycle, INPUT);
  pinMode(pushButtonToSendPayload, INPUT);
}

void loop() {  
  //If sendPayloadToSlaves variable is TRUE, it indicates a payload is to be sent to the slaves
  //If payloadTransmissionStatus is not TRUE, than last payload was unsuccesfully transmitted
  //and a new attempt should be made to transmit it again
  if (sendPayloadToSlaves || !payloadTransmissionStatus) {
    blinkLED(); //blink LEDs 3 times to indicate that an attempt to send the payload is being done
    payloadTransmissionStatus = radio.write( &sentPayload, sizeof(payloadRF) ); //Send payload sentPayload
    //payloadTransmissionStatus will store the transmission status: TRUE = succesful; FALSE = unsuccesful
    sendPayloadToSlaves = false; //One attempt to transmit the payload has been done, so this can be set to FALSE
    //In the end, LEDs will be off until a new cycle is selected with the push button
  }

  //Check if the pushButtonToSelectCycle has been pushed
  //If it is pushed, we should switch the measurement cycle to be sent to the slaves
  buttonToSelectCycle_CurrentStatus = digitalRead(pushButtonToSelectCycle);
  if (buttonToSelectCycle_CurrentStatus) {
    //If the above evaluates to true, than a new pushing event has occurred
    //Change the status of the payload accordingly to user input
    //The push button will change from cycle 1 to 2, and 2 to 1, sequentially
    if (firstLoop == true) {
      //This is only for the first loop (pressing), so to initialize the variables
      sentPayload.buttonToActivateCycle1_CurrentStatus = true;
      firstLoop = false;
    } else {
      //If it is not the first loop, then we shoul just swipe the variable status
      sentPayload.buttonToActivateCycle1_CurrentStatus = !sentPayload.buttonToActivateCycle1_CurrentStatus;
      sentPayload.buttonToActivateCycle2_CurrentStatus = !sentPayload.buttonToActivateCycle2_CurrentStatus;
    }
    //LEDs will indicate which measurement cycle was chosen and that will be/was transmitted to the slaves
    digitalWrite(greenLED, sentPayload.buttonToActivateCycle1_CurrentStatus);
    digitalWrite(redLED, sentPayload.buttonToActivateCycle2_CurrentStatus);
  }
  delay(1000); //Delay to reduce the chance of consecutive pushes to the button be detected

  //Check if the pushButtonToSendPayload has been pushed
  //If it is pushed, we should indicate that a payload is ready to be sent
  buttonToSendPayload_CurrentStatus = digitalRead(pushButtonToSendPayload);
  if (buttonToSendPayload_CurrentStatus) {
    //Change the status of sendPayloadToSlaves button, so the payload can be sent
    sendPayloadToSlaves = true;
  }
  delay(1000); //Delay to reduce the chance of consecutive pushes to the button be detected
}

void blinkLED() {
  //Function to blink the greeLED and redLED three times with a 500 ms interval
  for (byte i = 0; i < 3; i++) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH);
    delay(500);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    delay(500);
  }
}
