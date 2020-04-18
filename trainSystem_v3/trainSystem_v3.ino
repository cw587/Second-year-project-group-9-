//Libraries needed for project
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <SD.h>

//Can define the first
#define arrayLength 20

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

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4

File myFile;
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN1, MAX_DEVICES);

String stations[arrayLength] = {};
byte state = LOW;
int endOfArray;

void loadNewStations()
{
  state = HIGH;
  while(!Serial.available()){digitalWrite(LED1, state); delay(50); state = !state;}
    for(int i = 0; i <= arrayLength ; i++)
    {
      String temp = Serial.readString();
      if(temp == "end")
      {
        endOfArray = i;
        i = arrayLength;
        while(Serial.readString() == temp){}
      }
      else
      {
        digitalWrite(LED2, state);
        state = !state;
        stations[i] = temp;
      } 
    }
}

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
    myFile = SD.open("sizeOfArray.txt", FILE_WRITE);
    myFile.println(endOfArray);
    myFile.close();
    digitalWrite(CS_PIN2, HIGH);
  }
}

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

  //Sets both LEDS
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(CS_PIN1, OUTPUT);
  pinMode(CS_PIN1, OUTPUT);
  
  dotMatrix.begin();
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
      myFile = SD.open("sizeOfArray.txt", FILE_READ);
      endOfArray = myFile.read();
      myFile.close();
      myFile = SD.open("stations.txt");
      for(int i = 0 ; i <= endOfArray; i++)
      {
        stations[i] = myFile.readString();
      }
      myFile.close();
  }
  
  //Sets both select pins to high so neither one is active
  digitalWrite(CS_PIN1, HIGH);
  digitalWrite(CS_PIN2, HIGH);
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
