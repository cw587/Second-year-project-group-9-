//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds

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
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW

//Defines how many MAX7219 dot matrices are there chained together
#define MAX_DEVICES 4

#define eAddress 10

//Defines the variables of the dotMatrix and configs it send signal to correct pins
 MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

//Voltaile variable to store current station
volatile byte currentStation, state = LOW;

//Stores the amount of stations loaded
byte maxStation;

char lineName[20];

//Clears EEPRROM memory of the values stored, mostly used for testing but may have an application in final project
void clearEEPROM()
{
  
  digitalWrite(LED1, HIGH);
  for (int i = 0 ; i < EEPROM.length() ; i++)
  {
    EEPROM.write(i, 0);
  }
  currentStation = 0;
  digitalWrite(LED1, LOW);
}

/*
 * ISR that marks when a new station is hit and sends the value of the current station to LabView as well increasing the
 * currentStation variable on the board, this variable then overwrites the part of the EEPROM memory that holds this value
 * this ensures if power fails it can find it's location again
*/
void increaseStation()
{
    state = HIGH;
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
  dotMatrix.setSpeed(5);
  dotMatrix.setPause(5);


  pinMode(button, INPUT);
  pinMode(LED1, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(frontSensor), increaseStation, FALLING);
  attachInterrupt(digitalPinToInterrupt(button), clearEEPROM, RISING);
  currentStation = EEPROM.read(1);
  maxStation = EEPROM.read(0);

  Serial.flush();
}

void loop() {

  dotMatrix.displayAnimate();
  if (Serial.available() > 0)
  {
    String temp = Serial.readString();
    if (temp == "start")
    {
      digitalWrite(LED1, HIGH);
      for (int i = 1; i <= 19 ; i++)
      {
        while (!(Serial.available() > 0)) {}
        String temp = Serial.readString();
        char tempChar[20];
        if (temp == "end")
        {
          maxStation = i - 3;
          EEPROM.update(0, maxStation);
          i = 19;
        }
        else
        {
          temp.toCharArray(tempChar, 20);
          EEPROM.put(((i * eAddress) * 2), tempChar);
        }
      }
      digitalWrite(LED1, LOW);
      for (int i = 1; i <= 19 ; i++)
      {
        char toPrint[20];
        EEPROM.get((((i + 1) * eAddress) * 2), toPrint);
        dotMatrix.print(toPrint);
        delay(2000);
      }
      EEPROM.get((((1) * eAddress) * 2), lineName);
    }
  }


  if(state == HIGH)
  {
    if (digitalRead(frontSensor) == 0 && digitalRead(backSensor) == 0)
    {
      dotMatrix.displayReset();
      Serial.println(currentStation);
      EEPROM.write(1, currentStation);
      currentStation++;
      char temp[20], temp2[100], temp3[] = " caling at ";
      EEPROM.get((((currentStation +1) * eAddress) * 2), temp);
      strcpy(temp2, lineName);
      strcat(temp2, temp3);
      strcat(temp2, temp);
      //temp2 = lineName + " calling at " + temp; 
      dotMatrix.displayText(temp2, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
      
      if (currentStation > maxStation)
      {
        Serial.println(currentStation + 1);
        currentStation = 0;
        EEPROM.write(1, currentStation);
      }
      digitalWrite(LED1, HIGH);
      while ((digitalRead(backSensor) + digitalRead(frontSensor)) <= 1) {}
      digitalWrite(LED1, LOW);
      state = LOW;
    }
  }
}
