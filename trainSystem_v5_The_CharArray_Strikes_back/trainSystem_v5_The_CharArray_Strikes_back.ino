//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

//Can define the first
#define arrayLength 20

//Defines characters array length
#define charLength 20

#define LED1 6
#define LED2 9
#define BUTTON1 3
#define BUTTON2 A3
#define BUTTON3 A1

#define CLK_PIN   13
#define DATA_PIN  11

//Chip select pin for display
#define CS_PIN1   A5
//Chip select pin for card reader
#define CS_PIN2   A4

//Defines characteristics of display
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW

//Defines how many MAX7219 dot matrices are there
#define MAX_DEVICES 4

File myFile;
//Cr
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN1, MAX_DEVICES);

char stations[arrayLength][charLength];
byte state = LOW;
int endOfArray;

/*
 * This function was made to load new functions into the volatile flash memory of the
 * Arduino. It works by waiting for data over the Serial port (from labview), once received
 * it receives the data which will be the stations names it then adds it to an index of the
 * stations array (index decided by iteration number of for loop).
 * 
 * If the data incoming is equal to end then the values of the loop is defined as the
 * endOfArray and then the iteration variable is set to the max of the for loop to disable it,
 * a while loop runs after until the string disappears from the serial port.
 * 
 */
void loadNewStations()
{
  state = HIGH;
  while(!Serial.available()){digitalWrite(LED1, state); delay(50); state = !state;}
    for(int i = 0; i <= arrayLength ; i++)
    {
      char temp[charLength];
      Serial.readString().toCharArray(temp, charLength);
      if(temp == "end")
      {
        endOfArray = i;
        EEPROM.write(0, i);
        i = arrayLength;
        while(Serial.readString() == temp){}
      }
      else
      {
        digitalWrite(LED2, state);
        state = !state;
      } 
      stations[i][charLength] = temp;
    }
}

/*
 * This function first checks if the range of the staions array is known, if not
 * then stations must not of been loaded yet. Then turns the select pin for the SD
 * card reader to low to enable SPI connection
 * 
 * The next stage is to open a text file called stations.txt which is file in which
 * the station arrays will be saved, this then turns on an LED to confirm this. Then
 * the station array is iterated through and each one is saved into the sd card with
 * a delay to stop errors
 * 
 * Finally the stations.txt file is closed and then a new file is opened called
 * sizeOfArray.txt to store the size of the array just saved before closing it and 
 * changing the select channel to high to disable SPI connection
 */
void writeStations()
{
  if(endOfArray != NULL)
  {
    digitalWrite(CS_PIN2, LOW);
    myFile = SD.open("stations.txt", FILE_WRITE);
    digitalWrite(LED1, HIGH);
    for(int i = 0; i <= endOfArray; i++)
    {
      myFile.println(stations[i]);
      delay(100);  
    }
    myFile.close();
    //insert size of array
    myFile = SD.open("sizeOfArray.txt", FILE_WRITE);
    myFile.println(endOfArray);
    myFile.close();
    digitalWrite(CS_PIN2, HIGH);
  }
}

/*
 * This function is run when the user wishes to test to see what values are
 * stored in the stations array, this function iterates through each value of 
 * the array printing each one for a second. This function only runs if
 * the end of the array is known, if absent it means no stations have been
 * loaded
 */
void simulation()
{
  if(endOfArray != NULL)
  {
    for(int i = 0; i <= endOfArray ; i++)
    {
      dotMatrix.print(stations[i]);
      delay(1000);  
    }  
  } 
}


void setup() {

  //Sets both LEDS as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  //Sets both buttons to inputs starting at high values
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  //Sets both Select channels as outputs
  pinMode(CS_PIN1, OUTPUT);
  pinMode(CS_PIN1, OUTPUT);

  //Activates the dot matrix
  dotMatrix.begin();
  //Activates the serial port
  Serial.begin(9600);

  //Starts the SD card on the select channel chosen and stops the program if
  //it fails
  if (!SD.begin(!CS_PIN2))
  {
    Serial.println("initialization failed!");
    while (1){digitalWrite(LED1, state); digitalWrite(LED2, !state); state = !state; delay(200);}
  }

  //If the SD already contains data then it loads it into the stations array
  if(SD.exists("stations.txt"))
  {
      myFile = SD.open("stations.txt", FILE_READ);
      for(int i = 0 ; i <= endOfArray; i++)
      {
        char temp[charLength] = {myFile.read()};
        stations[i][charLength] = temp;
      }
      myFile.close();
  }
  
  //Sets both select pins to high so neither one is active
  digitalWrite(CS_PIN1, HIGH);
  digitalWrite(CS_PIN2, HIGH);

  endOfArray = EEPROM.read(0);
}

void loop() 
{
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  if(digitalRead(BUTTON1) == LOW)
  {
    loadNewStations();
  }
  else if(digitalRead(BUTTON2) == LOW)
  {
    simulation();
  }
  else if(digitalRead(BUTTON3) == LOW)
  {
     writeStations();
  }
}
