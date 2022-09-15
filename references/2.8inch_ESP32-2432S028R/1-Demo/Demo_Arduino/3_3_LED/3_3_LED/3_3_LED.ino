#include <Arduino.h>

int freq = 2000;    // frequency
int channel = 0;    // aisle
int resolution = 8;   // Resolution

const int led = 4;
void setup()
{
  //Initialize GPIO, turn off tricolor light

  pinMode(4, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(4, 0);
  digitalWrite(16, 0);
  digitalWrite(17, 0);
  ledcSetup(channel, freq, resolution); // set channel
  //ledcAttachPin(led, channel);  // Connect the channel to the corresponding pin
}

void loop()
{
  digitalWrite(4, 0);
  digitalWrite(16, 1);
  digitalWrite(17, 1);
  delay(500);
  digitalWrite(4, 1);
  digitalWrite(16, 0);
  digitalWrite(17, 1);
  delay(500);
  digitalWrite(4, 1);
  digitalWrite(16, 1);
  digitalWrite(17, 0);
  delay(500);
  digitalWrite(4, 1);
  digitalWrite(16, 1);
  digitalWrite(17, 1);
  delay(500);
  ledcAttachPin(led, channel);
  // gradually brighten
   for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle = dutyCycle - 5)
  {
    ledcWrite(channel, dutyCycle);  // output PWM
    delay(100);
  }
  // gradually darken
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle = dutyCycle + 5)
  {
    ledcWrite(channel, dutyCycle);  // output PWM
    delay(100);
  }
  delay(500);
}
