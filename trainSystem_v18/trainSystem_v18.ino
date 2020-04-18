//EEPROM stands for Electronically Erasable Programmable Memory

//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>

//Sets the effects, speed and pause for the animation of text
uint8_t scrollSpeed = 20;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
int16_t scrollPause = 2500; // in milliseconds

//defining display SPI pins due to MEGA not using default UNO pins for SPI
#define DATA_PIN 51
#define CLK_PIN 52
#define CS_PIN 53

//Define pull up button input for testing
#define button 18

//Testing LED for testing
#define LED1 8

//defning sensor pins for the train station
#define frontSensor 2
#define backSensor 3

//Defines the type of dot matrix display that is being used (Check library file to find other types of available dot matrix configs)
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

//Defines how many MAX7219 dot matrices are there chained together
#define MAX_DEVICES 6

//A value to multiple by current station to get the 
#define eAddress 20

//Defines the pins the speaker is connected to
#define SpeakerPin 4

//Defines the variables of the dotMatrix and configs it send signal to correct pins
 MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

//Voltaile variable to store current station
volatile byte currentStation, state = LOW;

//Stores the amount of stations currently being used
byte maxStation;

//A character array that has the name of the line being used loaded in setup()
char lineName[20];

////int TotalDurationMillis = 4000;
////
////void PlayTone() {
////  //int DurationMillis = Duration;// * 1000;
////  int Notes[] = {380, 440, 370, 328, 410, 460};
////  float DurationPerenctage[] = {0.2, 0.3, 0.2, 0.1, 0.1, 0.1};
////  int Duration = 0;
////  int x = 0;
////  
////  for (x = 0; x < 6; x++) {
////    Duration = DurationPerenctage[x] * TotalDurationMillis;
////    tone(SpeakerPin, Notes[x], Duration);
////    delay(Duration);
////    noTone(SpeakerPin);
////  }
////}
//


/*
 * This ISR is attached to pin2 the first sensor on the train station, when the train passes it changes STATE to HIGH
 * which will then trigger a statement in the loop this was due to the length of the statement to change station
 * on the display
*/
void increaseStation()
{
    state = HIGH;
}

//Function that puts a character array of any length onto the dotmatrix and animates it's printing
void displayPrint(char input[])
{
    dotMatrix.displayText(input, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);  
}


/*
 * The setup first starts the serial and dotMatrix communication and sets pin values of the button and LED
 * it also attachs two interrupts, the first to pin 2 that is connected to a reflective IR sensor meaning that when the train pulls into the station 
 * it runs a ISR the
 */
void setup() {

  //Starts the communication of Arduinoa and serial port/dotmatrix
  Serial.begin(9600);
  dotMatrix.begin();

  /*Attaches a falling interrupt to the sensor on pin 2 of the train station that only trigers when falling due
   * to how IR reflective sensors being high when not object is present  */
  attachInterrupt(digitalPinToInterrupt(frontSensor), increaseStation, FALLING);

  //Loads current station, the range of the stations and line name into variables again in case of power failure
  currentStation = (EEPROM.read(1) + 2);
  maxStation = EEPROM.read(0);
  EEPROM.get(((1) * eAddress), lineName);
  
  //Recreates the current station string with the line name in case of power failure
  char startArray[20], temp2[100];
  EEPROM.get(((currentStation) * eAddress),startArray);
  strcpy(temp2, lineName);
  strcat(temp2, " calling at ");
  strcat(temp2, startArray);
  displayPrint(temp2);

  //Sets the displays 0 to 5 to be zone 0
  dotMatrix.setZone(0, 0, 5);

  //Flushes the serial port to stop false reads from previous tests
  Serial.flush();
}

void loop() {

  //Makes loaded text animated on the dotMatrix
  dotMatrix.displayAnimate();

  /*
   * Checks if there is a value available on the serial port and if there is it checks if the value is equal
   * to "start", if so this is an indicator that the labview application has started loading new character arrays.
   * The dot matrix is then cleared to stop any graphical glitches that look unpleasant, a for loop is then started
   * that runs 18 times (the maximum set of strings allowed to be loaded from labview). 
   * 
   * Each iteration starts waiting for a value to appear on the serial port when this happens it is then read to 
   * a temporary string and then converted to a character array that is 20 bytes (meaning each station) can only be
   * 20 bytes max. It then puts the temporary character array into the EEPROM memory, it determines the location by
   * using the index of the for loop and then multiples it by eAddress definition at the start.
   * 
   * This means that at 20 bytes the line name will be stored, then from 40 to 20 * 17 the staions names will be stored 
   * in 20 byte chunks. If the value coming in is equal to "end" then the current index is adjusted and saved to mark
   * the size of the station so not all 16 places must be filled, this is then put in EEPROM and then the for loop index
   * is changed to trigger the end of the loop, finally the EEPROM value and the variable value for the current station are
   * then set to 0. Once the for loop has ended the EEPROM gets the value stored at 20 bytes which will always be
   * where the line name is stored
   */
  if (Serial.available() > 0)
  {
    String temp = Serial.readString();
    if (temp == "start")
    {
      dotMatrix.displayClear();
      for (int i = 1; i <= 19 ; i++)
      {
        while (!(Serial.available() > 0)) {}
        String temp = Serial.readString();
        char tempChar[20];
        if (temp == "end")
        {
          maxStation = i - 4;
          EEPROM.update(0, maxStation);
          i = 19;
          currentStation = 0;
          EEPROM.write(1, currentStation);
        }
        else
        {
          temp.toCharArray(tempChar, 20);
          EEPROM.put((i * eAddress), tempChar);
        }
      }
      EEPROM.get(((1) * eAddress), lineName);
    }
  }

  /*
   * Once the first sensor on the train station is passed it triggers an ISR that changes state to HIGH to mark that
   * the train has passed this station, this is then verfied in the below if statement. It then clears the dotMatrix
   * display and then gets the new station name and assigns it to a temporary char array. After the second temp char array
   * (temp2) copies the line name and then concatenates important information into the char array[] to make it readable
   * for the passenger. When the Char array temp2 has all the information needed it is then used as a parameter in the
   * displayPrint() statement that prints it to the dotmatrix.
   * 
   * Current station is then incremented only if it is not greater than the max station (if true it means the final station
   * has been reached). If it will be printed to the serial port for the labview application to display on the PC side that
   * a staition has been passed then the variable currentStation will be incremented by 1 and then saved to EEPROM memory
   * (in case of power failures).
   * 
   * If the currentStation is found to be the end then the current station variable will be set back to 1 so it loads
   * the first station again next time instead of incremented past its boundaries. To stop accidental double reads the statement
   * will only end if both sensors return back to values when an object is not presented. The state is turned back to low waiting 
   * for the next interrupt and a delay of 500 milliseconds to stop errors with displayAnimate().
   */
  if(state == HIGH)
  {
    if (digitalRead(frontSensor) == 0 && digitalRead(backSensor) == 0)
    {
      dotMatrix.displayClear();

      char temp[20], temp2[100];
      
      EEPROM.get(((currentStation +1) * eAddress), temp);
      
      strcpy(temp2, lineName);
      strcat(temp2, " calling at ");
      strcat(temp2, temp);
      displayPrint(temp2);
      
      if (currentStation > maxStation + 1)
      {
        currentStation = 1;
        EEPROM.write(1, currentStation);
      }
      else
      {
        Serial.println(currentStation);
        EEPROM.write(1, currentStation);
        currentStation++;  
      }
      while (digitalRead(backSensor) == 0 || digitalRead(frontSensor) == 0) {}
      state = LOW;
      delay(500);
    }
  }

  /*
   * Checks to see if the current animation cycle for the station is complete, if so it resets the display
   * so it will print once again.
   */
  if(dotMatrix.displayAnimate() == 1)
  {
    dotMatrix.displayReset();  
  }

}
