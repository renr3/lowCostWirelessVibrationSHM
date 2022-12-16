//Libraries
//See documentation at https://nRF24.github.io/RF24
#include <RF24.h>
#include <SPI.h> //SPI communication library
#include <SD.h> //SD card module library
#include <Wire.h> //I2C communication library
#include <DS3231.h> //RTC module library


//A struct variable to hold data that will be transmitted through the radio
//It is essentially the payload that will be always transmitted
struct structPayloadRF
{
  //This variable will indicate for slaves to perform measurement cycle 1
  boolean buttonToActivateCycle1_CurrentStatus = false;
  //This variable will indicate for slaves to perform measurement cycle 2
  boolean buttonToActivateCycle2_CurrentStatus = false;
};
//Definition of the type payloadRF as a struct of the type structPayloadRF
typedef struct structPayloadRF payloadRF;
//Creation of a payload variable to receive data
payloadRF receivedPayload;
//Constructor of the RF24 module
RF24 radio(7, 8);
byte address[][6] = {"1node"};

//Sampling parameters
int samplingFrequency_Cycle1 = 300; //In Hz
int samplingFrequency_Cycle2 = 300; //In Hz
int samplingFrequency_Current = 0; //In Hz
unsigned long samplingDuration_Cycle1 = 300; //Sampling duration in seconds of measurement Cycle 1
unsigned long samplingDuration_Cycle2 = 3000; //Sampling duration in seconds of measurement Cycle 2
unsigned long samplingDuration_Current = 0; //Sampling duration in seconds of current measurement cycle
unsigned long samplingStartTime = 0; //Variable to know when sampling started, to allow for sampling stop
long accelX, accelY, accelZ; //Variables to store acceleration data

//Other variables used in the code
File myFile; //Variable to store the file to receive the data
int x = 0; //Variable to store the file naming
DS3231 rtc(SDA, SCL); //Constructor of the RTC module
Time t; //Variable to store the time measurement DS3132.h

//Define LED and button pins
#define greenLED A0
#define redLED A1
byte resistorNoBreak[4] = {5, 6, 9, 10}; //Resistor to have a minimum current draw avoid powerbank to shutdown
//Variable to control SPI active channel, as both RF24 and SD card module use it
#define selectPinRF 7
#define selectPinSD 4


void setup() {
  //Set no-break resistors to provide a minimum current draw so powerbank doesn't turn off
  for (byte i = 0; i <= 4; i++) {
    pinMode(resistorNoBreak[i], OUTPUT);
    digitalWrite(resistorNoBreak[i], HIGH);
  }

  //Initialize the RTC module
  rtc.begin();

  //Initialize MPU6050 module
  Wire.begin();
  setupMPU();

  //Select SD on SPI bus
  selectSDonSPIbus();
  //Initialize SD module connections
  while (!SD.begin(selectPinSD)) {
    //If SD module doesn't begin, the system will just freeze
    //The LED will be constantly blinking to show an error state
    blinkLED(1);
  }
  delay(50); //Little pause so everything goes well

  //Select RF on SPI bus and start the module
  selectRFonSPIbus();
  radio.begin(); //Initialize RF24 module
  radio.openReadingPipe(1, address[0]); //Configure RF24 module to read on the pipe indicated on address[0]. This will be pipe #1
  radio.startListening(); //Start listening for messages from Master
}

void loop() {
  //Listen to Master
  listenToMaster();
  
  //Check the status of receivedPayload variable to see if either cycle must begin
  if (receivedPayload.buttonToActivateCycle1_CurrentStatus == true || receivedPayload.buttonToActivateCycle2_CurrentStatus == true) {
    //If the buttonToActiveCycle1_CurrentStatus is now true, than a Cycle 1 measurement should be performed
    //If the buttonToActiveCycle2_CurrentStatus is now true, than a Cycle 2 measurement should be performed
    //Just one of them will be true each time, never both

    //Activate LED to indicate which cycle is being performed and set proper variables
    if (receivedPayload.buttonToActivateCycle1_CurrentStatus == true) {
      digitalWrite(greenLED, HIGH); //Set greenLED high so we can see Cycle 1 has started
      samplingDuration_Current = samplingDuration_Cycle1; //Set current sampling duration
      samplingFrequency_Current = samplingFrequency_Cycle1; //Set current sampling frequency
    } else {
      digitalWrite(redLED, HIGH); //Set redLED high so we can see Cycle 2 has started
      samplingDuration_Current = samplingDuration_Cycle2; //Set current sampling duration
      samplingFrequency_Current = samplingFrequency_Cycle2; //Set current sampling frequency
    }
    
    selectSDonSPIbus();
    createNewLogFile(); //Open new file
    t = rtc.getTime(); //Get the current time
    
    //Print data about the measurement session being performed
    myFile.println();
    myFile.println(F("Measurement Cycle 1"));
    myFile.print(F("Sampling duration: "));
    myFile.print(samplingDuration_Current);
    myFile.println(F(" seconds."));
    myFile.print(F("Sampling frequency: "));
    myFile.print(samplingFrequency_Current);
    myFile.println(F(" Hz."));
    myFile.println();
    saveClockDataToFile();

    //Start measurement session
    samplingStartTime = millis(); //Get time of beginning of measurement
    while (millis() - samplingStartTime < samplingDuration_Current * 1000) {
      acquireAccelerometerData(); //Get accelerometer data
      saveAccelerometerDataToFile(); //Save accelerometer values in the text file inside the SD card
      delayMicroseconds(1000000 / samplingFrequency_Current); //Wait the correct time for next measurement
    }
    //Finish measurement session
    myFile.close();

    //Finish measurement cycle
    if (receivedPayload.buttonToActivateCycle1_CurrentStatus == true) {
      digitalWrite(greenLED, LOW); //Turn off LED to indicate to user measurement cycle has finished
      receivedPayload.buttonToActivateCycle1_CurrentStatus = false;
    } else {
      digitalWrite(redLED, LOW); //Turn off LED to indicate to user measurement cycle has finished
      receivedPayload.buttonToActivateCycle2_CurrentStatus = false;
    }    
    delay(1000); //A little delay so everything runs smooth
  }
}

void selectSDonSPIbus() {
  //Select SD on SPI bus
  digitalWrite(selectPinSD, LOW);
  digitalWrite(selectPinRF, HIGH);
  delay(100); //Little pause so everything goes well
}

void selectRFonSPIbus() {
  //Select RF on SPI bus and start the module
  digitalWrite(selectPinSD, HIGH);
  digitalWrite(selectPinRF, LOW);
  delay(100); //Little pause so everything goes well
}

void createNewLogFile() {
  //Create a new file, sequentially to the one already created.
  //Iterate through all the files in the SD card
  while (SD.exists("File" + (String)x + ".txt")) {
    //If the file exists, increase x variable
    x++;
  }
  //Make a small pause so to try opening the next file
  delay(500);
  //Open the next file based on the next name available found
  myFile = SD.open("File" + (String)x + ".txt", FILE_WRITE);
}

void listenToMaster() {
  selectRFonSPIbus();
  //Function that will check if there is new message from the Master
  if (radio.available()) {
    //A new message was detected. Read it and store on receivedPayload
    radio.read( &receivedPayload, sizeof(payloadRF) );
  }
}

void blinkLED(byte numberBlinks) {
  //Function to blink the greeLED and redLED in a numberBlinks times with a 500 ms interval
  for (byte i = 0; i < numberBlinks; i++) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH);
    delay(500);
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH);
    delay(500);
  }
}

void setupMPU() {
  Wire.beginTransmission(0b1101001); //I2C address of the device. If pin AD0 is LOW, address is 0b1101000, if AD0 is HIGH, address is 0b1101001
  Wire.write(0x6B); //Register 6B refers to chip's power management
  Wire.write(0b00000000); //No sleep or cyclic modes - 4.28 Register Map
  Wire.endTransmission(); //End I2C transmission.
  Wire.beginTransmission(0b1101001);//I2C address of the device.
  Wire.write(0x1C); //Accesing the accelerometer configuration (0x1b to access gyroscopes configuration)
  Wire.write(0b00000000); //Set acceleration range to +-2g - 4.5 Register Map
  Wire.endTransmission(); //End I2C transmission.
}

void acquireAccelerometerData() {
  //Code to get data from the MPU6050. Chexk address to see if it fits your model!
  Wire.beginTransmission(0b1101001);
  Wire.write(0x3B); //4.17 Register Map
  Wire.endTransmission();
  Wire.requestFrom(0b1101001, 6);
  while (Wire.available() < 6);
  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();;
}

void saveClockDataToFile() {
  //Register the time in the file
  myFile.println("Hora: RUNTIME (micros)" + String(micros()) + ", RTC HOUR (h)" + String(t.hour) + ", RTC MINUTES (m)" + String(t.min) + ", RTC SECONDS (s)" + String(t.sec));
}

void saveAccelerometerDataToFile() {
  //Register the acceleration data in the file
  myFile.println(String(micros()) + "," + (accelX) + "," + (accelY) + "," + (accelZ));
}
