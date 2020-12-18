/******************************************************************************
 * A temperature controlling program used to rise bread, written for for      *
 * Arduino UNO. This is currently configured to use a TMP36 Temp sensor and   *
 * the TM1637 based 4 digit seven segment display with one colon in the       *
 * center. Note all temperature values are in celcius. Below there is a       *
 * Configuration Variables section to set the commonly used timings. These    *
 * are mainly for debugging but can be tweaked to your desire.                *
 ******************************************************************************
 * (C) 2020 Autumn Bauman, All rights reserved. github.com/narg3000           *
 * This code is licensed under the GNU Gneral Public License Version 2.       *
 * See gnu.org/licenses for more details                                      *
 ******************************************************************************
 * Version Number: 0.1.1, testing failed horribly, possibly due to my lack of *
 * a temperature probe, so I am outputting status through serial.             */
 
#include <TM1637Display.h>

// The pins used are declared below

const int TempSen = 0;    // Analog input of the Temp Sensor
const int TempUp = 2;     // Button to up the targetTemp
const int TempDown = 4;   // Button to decrease the targetTemp
const int TempSet = 3;    // Switch to change into the changeTemp() function
const int RelayPin = 6;   // Pin to control the relay to turn off and on the heater
const int Clck = 8;       // Clock pin for the TM1637
const int DispDat = 9;    // DIO pin on the TM1637

// Global Variables used by multiple functions.
 
int targetTemp;      // Target Temp.
int cycleCount;      // Cycle count used in holdTemp() function. Set in void setup()

/***************************************************************************
 *  Configuration variables to set delays for button input, heating times, *
 *  and initial configurations. Feel free to change these, bearing in mind *
 *  it will change the timing of the program. Intende for debugging        *
 **************************************************************************/
const int ButtonTime = 100;   // The time delay after each button input in ms 
                              // (see the changeTemp function for usage)
const int DisplayRate = 100;  // Time in ms between display refreshes, mainly used
                              // in the holdTemp() function
const int HeatTime = 5000;    // Time (in ms) that the heater will turn on for 
                              // before rechecking the current temperature
int InitTargetTemp = 26;      // The target temperature in Celcius used initally. 
                              // Set at 26 by default to minimise the work needing to be done by users
const int Brightness = 7;     // Display brigntness, 7 seems to work well on my display

 

//  Configureing the display librairy
TM1637Display display(Clck, DispDat); 

void setup()
{
  pinMode(TempUp, INPUT);
  pinMode(TempDown, INPUT);
  pinMode(TempSet, INPUT);
  pinMode(RelayPin, OUTPUT);
  display.setBrightness(Brightness);     // Set display brightness

  targetTemp = InitTargetTemp;  // Set target temp to what is specified in config section

// This code is used to configure the for() loop in the holdTemp function:
  cycleCount = HeatTime / DisplayRate;

  // Serial Debugging stuff added in v0.1.1
  Serial.begin(9600);
  Serial.println("Setup Complete. Entering Main...");
}

/* The main loop function, mainly used to call other functions and check if 
 *  any inputs are being received 
 */


void loop()
{
  Serial.println("Main Entered");
  int temp = readTemp();
  Serial.print("Temp = ");
  Serial.println(temp);
  if (digitalRead(TempSet) == HIGH){
    Serial.println("Entering changeTemp()");
    changeTemp();
  }
  
  if (readTemp() < targetTemp){
    Serial.println("Temp low, entering holdTemp()...");
    holdTemp();
  }
  // This line is repeated a lot because I cannot run it on loop in the background
  // The formatting is a bit fdunky, but it works
  display.showNumberDecEx(((targetTemp * 100) + readTemp()),  0b01000000);
  delay(DisplayRate);
}

/* Function to update the targetTemp variable
 *  
 * When TempSet switch is flicked, this function takes the inputs of 
 * the TempUp and TempDown inputs and adjusts the targetTemp variable 
 * accordingly. It also updates the display. The while loop contains 
 * the entire function.
 */    

void changeTemp(){
  Serial.println("changeTemp entered...");
  while(digitalRead(TempSet) == HIGH){    
    Serial.print("targetTemp = ");
    Serial.println(targetTemp);
    if(digitalRead(TempUp) == HIGH){   // Increases the temperature by one
      ++targetTemp;
      display.showNumberDecEx(((targetTemp * 100) + readTemp()),  0b01000000);
      Serial.println("Temp Up...");
      delay(ButtonTime);          // I added this delat statement to make sure that the button if held down, temp
                                  // doesn't shoot up too fast, and also to make sure that there is incramenting
    }                             // between button presses. This should negate the need for a debounce circuit
    
    if(digitalRead(TempDown) == HIGH){    // Decreases the temperature by one
      --targetTemp;
      display.showNumberDecEx(((targetTemp * 100) + readTemp()),  0b01000000);
      Serial.println("Temp Down...");
      delay(ButtonTime);
    }
    else{
      display.showNumberDecEx(((targetTemp * 100) + readTemp()),  0b01000000);
      Serial.println("No input...");
      delay(DisplayRate);
    }
  }
  Serial.println(
}

/* Function to gather the analog input from the thermocouple and return
 * the temperature in Celcius.
 */
int readTemp(){
  // Variables used in the function. Input is raw analog data, voltage is 
  // analog data converted into a voltage, and temp is the final temp used
  float input;
  float voltage;
  int temp;
  Serial.println("Reading Temp...");
  input = analogRead(TempSen);
  voltage = input * 5;
  temp = (voltage - 0.5) * 100;
  return temp;
}

/* function to keep everything at the right temperature by turning on the heater
 */

void holdTemp(){
  Serial.println("Entered holdTemp...");
  while(readTemp() < targetTemp){
    digitalWrite(RelayPin, HIGH);
    Serial.println("Relay is ON");
    
    for(int i = 0; i >=cycleCount; i++){
      Serial.print("Loop Iteration: ");
      Serial.println(i);
      display.showNumberDecEx(((targetTemp * 100) + readTemp()),  0b01000000);
      delay(DisplayRate);
      
       /* Checking if the user wants to change the temp.
        * In order to prevent potentially dangerous overheating,
        * the relay is turned off before exiting to the changeTemp 
        * function, and then the for loop breaks 
        */
       if (digitalRead(TempSet) == HIGH){
        Serial.println("Entering changeTemp...");
        digitalWrite(RelayPin, LOW);
        Serial.println("Relay off.");     
        changeTemp();   // 
        break;
       }
       delay(DisplayRate);
    }
  }
  digitalWrite(RelayPin, LOW);
}
