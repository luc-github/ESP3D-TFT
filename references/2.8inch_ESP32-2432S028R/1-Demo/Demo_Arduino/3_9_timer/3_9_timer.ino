/* 创建硬件定时器 */
hw_timer_t * timer = NULL;
 
/* LED 引脚 */
int led = 4;
/* LED 状态 */
volatile byte state = LOW;
 
void IRAM_ATTR onTimer(){
  state = !state;
  digitalWrite(led, state);
}
 
void setup() {
  Serial.begin(115200);
   //Initialize GPIO, turn off blue and green lights

  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  digitalWrite(16, 1);
  digitalWrite(17, 1);
  //Initialize red light
  pinMode(led, OUTPUT);
 
  
  /*  1/(80MHZ/80) = 1us  */
  timer = timerBegin(0, 80, true);
 
  /* Attach the onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);
 
  /* *Set the alarm clock to call the onTimer function every second 1 tick is 1us => 1 second is 1000000us * / 
  / *Repeat alarm (third parameter)*/
 
  timerAlarmWrite(timer, 1000000, true);
 
  /*Start alert*/
  timerAlarmEnable(timer);
  Serial.println("start timer");
}
 
void loop() {
 
}
