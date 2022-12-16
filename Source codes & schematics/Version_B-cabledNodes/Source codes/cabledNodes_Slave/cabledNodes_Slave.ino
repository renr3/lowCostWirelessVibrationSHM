//Libraries
#include <SoftwareSerial.h> //RS485 library
#include <SPI.h> //SPI communication library
#include <Wire.h> //I2C communication library
#include <SD.h> //SD card module library
#include <DS3231.h> //RTC module library

//Pin numberings
#define RXpin 6
#define TXpin 5
#define LED 2 //LED state
#define resistorBreak1 9
#define resistorBreak2 7
#define resistorBreak3 10

//RS485 module definitions
#define serialControl 3
#define RS485transmit    HIGH
#define RS485receive     LOW
SoftwareSerial RS485Serial(RXpin, TXpin); //Constructor of the RS485 module

//Variables used in the code
String inputString = ""; //Variable to store variable coming from RS485 module
long accelX, accelY, accelZ; //Variables to store acceleration data
int slaveGo = 0; //Variable to control the sampling cycles
int x = 0; //Variable to store the file naming
File myFile; //Variable to store the file to receive the data

DS3231 rtc(SDA, SCL); //Constructor of the RTC module
Time t; //Variable to store the time measurement DS3132.h

//Sampling parameters
int samplingFrequency = 0; //In Hz
unsigned long samplingDuration = 600; //Sampling duration in seconds
unsigned long samplingStartTime = 0; //Variable to know when sampling started, to allow for sampling stop

void setup() {
  //Set the resistor pins to output.
  //These pins are used to keep the external power bank on
  pinMode(resistorBreak1, OUTPUT);
  pinMode(resistorBreak2, OUTPUT);
  pinMode(resistorBreak3, OUTPUT);
  digitalWrite(resistorBreak1, HIGH);
  digitalWrite(resistorBreak2, HIGH);
  digitalWrite(resistorBreak3, HIGH);

  //Set LED and RS485 pins
  pinMode(LED, OUTPUT);
  pinMode(serialControl, OUTPUT);

  //Initialize the RTC module
  rtc.begin();

  //Initialize MPU6050 module
  Wire.begin();
  setupMPU();

  //Set serial connections
  Serial.begin(9600);
  RS485Serial.begin(4800);
  digitalWrite(serialControl, RS485receive); //Set the RS485 to receiver mode

  //Initiate serial connections
  while (!Serial) {
    //If serial port doesn't begin, the system will just freeze
    //The LED will be constantly blinking to show an error state
    blinkLED();
  }

  //Initialize SD module connections
  while (!SD.begin(4)) {
    //If SD module doesn't begin, the system will just freeze
    //The LED will be constantly blinking to show an error state
    blinkLED();
  }
}

//This loop will wait for master to send a signal and make a measurement
void loop() {
  //Check if there is incoming message from Master
  if (RS485Serial.available()) {
    inputString += (char)RS485Serial.read();
    if (inputString == '1') {
      //If inputString is 1, the master node told slave to start a measurement session
      inputString = ""; //Reset tge inputString variable
      slaveGo = 1; //Update slaveGo variable to indicate a new measurement session to start
      delay(100); //A little delay so things go well
      RS485Serial.flush(); //Flush all serial data from RS485 channel
    }
  }
  //While slaveGo is 1, then we should perform a measurement session
  if (slaveGo == 1) {
    createNewLogFile(); //Open new file
    digitalWrite(LED, HIGH); //Turn on LED to indicate a measurement session is occuring
    t = rtc.getTime(); //Get the current time
    saveClockDataToFile(); //Register time
    samplingStartTime = millis(); //Get time of beginning of measurement
    while (millis() - samplingStartTime < samplingDuration * 1000) {
      acquireAccelerometerData(); //Get accelerometer data
      saveAccelerometerDataToFile(); //Save accelerometer values in the text file inside the SD card
      delayMicroseconds(1000000 / samplingFrequency); //Wait the correct time for next measurement
    }
    myFile.close();
    digitalWrite(LED, LOW);
    delay(3000);
    slaveGo = 0;
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

void blinkLED() {
  //Function to blink the LED one time with a 500 ms interval
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
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
  accelZ = Wire.read() << 8 | Wire.read();
}

void saveClockDataToFile() {
  //Register the time in the file
  myFile.println("Hora: RUNTIME (micros)" + String(micros()) + ", RTC HOUR (h)" + String(t.hour) + ", RTC MINUTES (m)" + String(t.min) + ", RTC SECONDS (s)" + String(t.sec));
}

void saveAccelerometerDataToFile() {
  //Register the acceleration data in the file
  myFile.println(String(micros()) + "," + (accelX) + "," + (accelY) + "," + (accelZ));
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
