//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
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
#define CS_PIN1   10

//Defines characteristics of display
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW

//Defines how many MAX7219 dot matrices are there
#define MAX_DEVICES 4

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
//void loadNewStations()
//{ 
//    bool ended = false;
//    for(int i = 0; i <= arrayLength ; i++)
//    {
//      while(!Serial.available() || ended == true){digitalWrite(LED1, state); delay(50); state = !state;}
//      char temp[charLength];
//      Serial.readString().toCharArray(temp, charLength);
//      if(temp == "end")
//      {
//        EEPROM.update(0, i);
//        i = arrayLength;
//        ended =true;
//        digitalWrite(LED2, HIGH);
//        
//        while(Serial.readString() == temp){}
//      }
//      else
//      {
//        EEPROM.put((i+1)*charLength, temp);
//        //digitalWrite(LED2, state);
//        state = !state;
//      }       
//    }
//}

void clearMemory()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i, 0);
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
    Serial.println(EEPROM.read(0));
    
    for(int i = 0; i <=EEPROM.read(0)  ; i++)
    {
      char temp[charLength];
      EEPROM.get((i+1)*charLength, temp);
      dotMatrix.print(temp);
      Serial.println(temp);
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
  else if(digitalRead(BUTTON3)==LOW)
  {
    clearMemory(); 
  }
}
