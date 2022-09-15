const byte interruptPin = 0;

volatile int interruptCounter = 0;

int numberOfInterrupts = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void setup() {
Serial.begin(115200);

Serial.println("Monitoring interrupts: ");
  //Turn off the LED
  pinMode(4,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  digitalWrite(4,1);
  digitalWrite(16,1);
  digitalWrite(17,1);
  //Interrupt pin
pinMode(interruptPin, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

}

void handleInterrupt() {
portENTER_CRITICAL_ISR(&mux);

interruptCounter++;

portEXIT_CRITICAL_ISR(&mux);

}

void loop() {
if(interruptCounter>0){
portENTER_CRITICAL(&mux);

interruptCounter--;

portEXIT_CRITICAL(&mux);

numberOfInterrupts++;

Serial.print("An interrupt has occurred. Total: ");
digitalWrite(4,!digitalRead(4));
digitalWrite(16,!digitalRead(16));
digitalWrite(17,!digitalRead(17));
Serial.println(numberOfInterrupts);
}

}
