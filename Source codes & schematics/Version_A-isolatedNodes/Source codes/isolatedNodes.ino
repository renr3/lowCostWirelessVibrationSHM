//Libraries
#include <SPI.h> //Library for SD Card module
#include <SD.h> //Library for SD Card module
#include <Wire.h> //Library for MPU6050
#include <DS3231.h> //Library by Henning Karlsen for RTC external clock. Downloaded from: http://www.rinkydinkelectronics.com/library.php?id=73

//Pin numberings
#define LED 2
#define buttonPin 8
#define resistorForPowerBank 9
#define resistorForPowerBank2 7
#define resistorForPowerBank3 10

//Global variables
boolean interfaceButton = LOW;
char stopSampling; //Comando para fechar o sd e terminar a aquisicao de dados quando conectado ao pc
int x = 0; //Variable to store the file naming
long accelX, accelY, accelZ; //Variables to store acceleration values in the 3 orthogonal axis

//Sampling parameters
int samplingFrequency = 0; //In Hz
unsigned long samplingDuration = 600; //Sampling duration in seconds
unsigned long samplingStartTime = 0; //Variable to know when sampling started, to allow for sampling stop

//SD card definitions
File myFile; //Constructor of the SD file object

//RTC clock definitions
DS3231 rtc(SDA, SCL); //Constructor of the RTC clock object
Time t; //Constructor of the time variable of DS3132.h library used in this skecth

void setup() {
  //Set the resistor pins to output. These pins are used to keep the external power bank on
  pinMode(resistorForPowerBank, OUTPUT);
  pinMode(resistorForPowerBank2, OUTPUT);
  pinMode(resistorForPowerBank3, OUTPUT);
  //Make the digital pins attached to resistors HIGH, so to keep a minimum current flow to
  //avoid the power bank powering the system to shut down. The resistors' values were computed
  //considering a maximum current of 40 mA in each pin, and a global maximum current of 200 mA.
  //By Ohm's Law considering each digital pin, when set to HIGH, provides 5V, a maximum current of 40 mA
  //corresponds to 125 ohms. So, resistors of at least 125 ohms were progressively added, in a trial and error
  //approach, to induce a minimum current draw that kept the power bank on.
  digitalWrite(resistorForPowerBank, HIGH);
  digitalWrite(resistorForPowerBank2, HIGH);
  digitalWrite(resistorForPowerBank3, HIGH);

  //Set the interface button to input, with the internal pullup resistor on.
  pinMode(interfaceButton, INPUT_PULLUP);

  //Initialize the RTC module
  rtc.begin();

  //Initialize accelerometer
  Wire.begin();
  setupMPU();

  //Initialize SD module connections
  while (!SD.begin(4)) {
    //If SD module doesn't begin, the system will just freeze
    //The LED will be constantly blinking to show an error state
    blinkLED();
  }
}

void loop() {
  //Keep checking if interfaceButton has been pressed, until it is pressed
  while (interfaceButton == LOW) {
    interfaceButton = digitalRead(buttonPin);
  }
  createNewLogFile(); //Create a new log file
  digitalWrite(LED, HIGH); //Indicate sampling has started
  t = rtc.getTime(); //Get the current time
  saveClockDataToFile(); //Register time
  samplingStartTime = millis(); //Get time of beginning of measurement
  while (millis() - samplingStartTime < samplingDuration * 1000) {
    acquireAccelerometerData(); //Get accelerometer data
    saveAccelerometerDataToFile(); //Save accelerometer values in the text file inside the SD card
    delayMicroseconds(1000000 / samplingFrequency); //Wait the correct time for next measurement
  }
  digitalWrite(LED, LOW); //Indicate sampling has finished
  myFile.close(); //Close file so a next file can be opened
  interfaceButton = HIGH;
  delay(3000); //A little delay so everything goes well
}

void setupMPU() {
  Wire.beginTransmission(0b1101001); //E o I2C address, se a porta AD0 esta LOW, sera 0b1101000, se AD0 esta HIGH, sera 0b1101001
  Wire.write(0x6B); //Registro 6B se refere a gestão de energia
  Wire.write(0b00000000); //Sem modos soneca ou ciclicos - 4.28 Register Map
  Wire.endTransmission();
  Wire.beginTransmission(0b1101001);//I2C address
  Wire.write(0x1C); //Acesso a configuracao dos acelerometros (0x1b - configuracao dos giroscopios)
  Wire.write(0b00000000); //Intervalo de aceleracao em +-2g - 4.5 Register Map
  Wire.endTransmission();
}

void blinkLED() {
  //Function to blink the LED one time with a 500 ms interval
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}

void acquireAccelerometerData() {
  Wire.beginTransmission(0b1101001);
  Wire.write(0x3B); //4.17 Regiter Map
  Wire.endTransmission();
  Wire.requestFrom(0b1101001, 6);
  while (Wire.available() < 6);
  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();
}

void saveClockDataToFile() {
  myFile.println("Hora: RUNTIME (micros)" + String(micros()) + ", RTC HOUR (h)" + String(t.hour) + ", RTC MINUTES (m)" + String(t.min) + ", RTC SECONDS (s)" + String(t.sec));
}

void saveAccelerometerDataToFile() {
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
