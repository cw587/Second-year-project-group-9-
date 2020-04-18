#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define stringLength 15
#define arrayLength 20

#define LED1 6
#define LED2 9
#define BUTTON 3

#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4


MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String stations[arrayLength] = {};
byte state = LOW;
int endOfArray;

void loadStations()
{
  while(!Serial.available()){digitalWrite(LED1, state); delay(50); state = !state;}
    for(int i = 0; i <= arrayLength ; i++)
    {
      
      String temp = Serial.readString();
      if(temp == "end")
      {
        endOfArray = i;
        i = arrayLength;
        P.print(temp);
        while(Serial.readString() == temp){}
      }
      else
      {
        stations[i] = temp;
        P.print(temp);
      } 
    }
}


void setup() {
  pinMode(BUTTON, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  P.begin();
  Serial.begin(9600);  
}

void loop() 
{

  if(digitalRead(BUTTON) == HIGH)
  {
    loadStations();
    P.displayClear();
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
  }
}
