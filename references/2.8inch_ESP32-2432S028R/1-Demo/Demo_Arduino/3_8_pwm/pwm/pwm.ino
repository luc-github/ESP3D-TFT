/********************************************/
/********************************************/
/********************************************/
const int backlightPin = 21;//Backlight Control

// PWM related parameter settings
int freq = 250;//2000
int channel = 0;
int resolution = 8;

/*******************************************/
void setup()
{
  Serial.begin(9600);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(backlightPin, channel);
  
}
/*******************************************/ 
void loop()
{
  //ledcWriteTone(channel, 250);//2000
  
  for (int dutyCycle = 0; dutyCycle <= 260; dutyCycle = dutyCycle + 20)
  {
    Serial.println(dutyCycle);
    ledcWrite(channel, dutyCycle);
    delay(300);
  }
  
  //ledcWrite(channel, 20);
  
  /*
  for(int i = 200;i <= 800;i++)   //频率循环从200到800
  {
    ledcWriteTone(channel,i);   //在pin17产生一个音调，它的频率是 i 变量
    delay(5);                   // 等待5毫秒
  }
  delay(4000);   //在最高频率上等待4秒
  for(int i = 800;i >= 200;i--)  //频率循环从800到200
  {
    ledcWriteTone(channel,i);  //在pin17产生一个音调，它的频率是 i 变量
    delay(10);  //等待10毫秒
  }
  */
}
