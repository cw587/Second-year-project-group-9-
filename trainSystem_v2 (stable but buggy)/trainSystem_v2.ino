#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <SD.h>

#define stringLength 15
#define arrayLength 20

#define LED1 6
#define LED2 9
#define BUTTON1 3
#define BUTTON2 A3
#define BUTTON3 A1

#define CLK_PIN   13
#define DATA_PIN  11
//For the display
#define CS_PIN1   A5
//For SD card reader
#define CS_PIN2   A4

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4

File myFile;
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN1, MAX_DEVICES);

String stations[arrayLength] = {};
byte state = LOW;
int endOfArray;

void loadStations()
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
    digitalWrite(CS_PIN2, LOW);
    myFile = SD.open("examples.txt", FILE_WRITE);
    for(int i = 0; i <= endOfArray; i++)
    {
      myFile.println(stations[i]);  
    }
    myFile.close();
    digitalWrite(CS_PIN2, HIGH);
}

void simulation()
{
  if(endOfArray != NULL)
  {
    for(int i = 0; i <= endOfArray ; i++)
    {
      P.print(stations[i]);
      delay(1000);  
    }  
  } 
}


void setup() {
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(CS_PIN1, OUTPUT);
  pinMode(CS_PIN1, OUTPUT);
  
  P.begin();
  Serial.begin(9600); 
  //SD.remove("example.txt");
  digitalWrite(CS_PIN1, HIGH);
  digitalWrite(CS_PIN2, HIGH);
}

void loop() 
{
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  if(digitalRead(BUTTON1) == LOW)
  {
    loadStations();
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
