//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>


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

#define eAddress 10

#define SpeakerPin 4

//Defines the variables of the dotMatrix and configs it send signal to correct pins
 MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

//Voltaile variable to store current station
volatile byte currentStation, state = LOW;

//Stores the amount of stations loaded
byte maxStation;

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
 * ISR that marks when a new station is hit and sends the value of the current station to LabView as well increasing the
 * currentStation variable on the board, this variable then overwrites the part of the EEPROM memory that holds this value
 * this ensures if power fails it can find it's location again
*/
void increaseStation()
{
    state = HIGH;
}

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
  // put your setup code here, to run once:

  Serial.begin(9600);
  dotMatrix.begin();

  pinMode(button, INPUT);
  pinMode(LED1, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(frontSensor), increaseStation, FALLING);
  currentStation = (EEPROM.read(1) + 2);
  maxStation = EEPROM.read(0);
  EEPROM.get((((1) * eAddress) * 2), lineName);
  char startArray[20], temp2[100];
  EEPROM.get((((currentStation) * eAddress) * 2),startArray);
  strcpy(temp2, lineName);
  strcat(temp2, " calling at ");
  strcat(temp2, startArray);
  displayPrint(temp2);
  dotMatrix.setZone(0, 0, 5);
  Serial.flush();
}

void loop() {

  
  dotMatrix.displayAnimate();
  if (Serial.available() > 0)
  {
    String temp = Serial.readString();
    if (temp == "start")
    {
      dotMatrix.displayClear();
      digitalWrite(LED1, HIGH);
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
          EEPROM.put(((i * eAddress) * 2), tempChar);
        }
      }
      EEPROM.get((((1) * eAddress) * 2), lineName);
    }
  }

  //dotMatrix.displayReset();

  if(state == HIGH)
  {
    if (digitalRead(frontSensor) == 0 && digitalRead(backSensor) == 0)
    {
      dotMatrix.displayClear();

      char temp[20], temp2[100];
      
      EEPROM.get((((currentStation +1) * eAddress) * 2), temp);
      
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
      digitalWrite(LED1, HIGH);
      while (digitalRead(backSensor) == 0 || digitalRead(frontSensor) == 0) {}
      //PlayTone();
      digitalWrite(LED1, LOW);
      state = LOW;
      delay(500);
    }
  }
  if(dotMatrix.displayAnimate() == 1)
  {
    dotMatrix.displayReset();  
  }

}
