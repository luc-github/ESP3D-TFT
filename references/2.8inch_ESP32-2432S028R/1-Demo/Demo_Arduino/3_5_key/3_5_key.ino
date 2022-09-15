//按键控制灯亮灭
void setup()
{
  //关LED
  pinMode(4,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  digitalWrite(4,1);
  digitalWrite(16,1);
  digitalWrite(17,1);
  //KEY
  pinMode(0,INPUT_PULLUP);
}

void loop()
{ 

  //Each time the key is pressed the RGB light turns on or off
  if(digitalRead(0))
  {
    while(digitalRead(0));
    digitalWrite(4,!digitalRead(4));
    digitalWrite(16,!digitalRead(16));
    digitalWrite(17,!digitalRead(17));
  }
}
