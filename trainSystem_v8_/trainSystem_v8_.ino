//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>

//defining channel select pins
#define CS 10

#define button1 2
#define button2 3
#define LED1 4

//Defines characteristics of display
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW

//Defines how many MAX7219 dot matrices are there
#define MAX_DEVICES 4

//Cr
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS, MAX_DEVICES);

volatile byte currentStation;

byte eAddress = 10, maxStation;

void clearEEPROM(int type)
{
  for (int i = 0 ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i, 0);
  }
  currentStation = 0;  
}

void increaseStation()
{
  
  Serial.println(currentStation);
  currentStation++;
  EEPROM.write(1, currentStation);
  delay(150);
  if(currentStation > maxStation)
  {
    currentStation = 0;
    EEPROM.write(1, currentStation);
  }  
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  dotMatrix.begin();

  
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(button1), increaseStation, FALLING);
  attachInterrupt(digitalPinToInterrupt(button2), clearEEPROM, FALLING);
  currentStation = EEPROM.read(1);
  maxStation = EEPROM.read(0);
 
  Serial.flush();
}

void loop() {
  if(Serial.available() > 0)
  {
    String temp = Serial.readString();
    if(temp == "start")
    {
      digitalWrite(LED1, HIGH); 
      for(int i = 1; i <= 19 ; i++)
      {
        while(!(Serial.available() > 0)){}
        String temp = Serial.readString();
        char tempChar[20];
        if(temp == "end")
        {
          maxStation = i - 2;
          EEPROM.update(0, maxStation);
          i = 19;
        }
        else
        {
          temp.toCharArray(tempChar, 20);
          EEPROM.put(((i*eAddress)*2), tempChar);
        }
      }
      digitalWrite(LED1, LOW);  
      for(int i = 1; i<=19 ; i++)
      {
        char toPrint[20];
        EEPROM.get(((i*eAddress)*2), toPrint);
        dotMatrix.print(toPrint);
        delay(1000);  
      }
    }

  }
}
