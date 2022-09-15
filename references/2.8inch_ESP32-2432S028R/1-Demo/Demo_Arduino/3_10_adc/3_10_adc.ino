#define ANALOG_PIN_0   34

void setup(){
  Serial.begin(115200);
  pinMode(ANALOG_PIN_0,INPUT);
}

void loop() {
  int analog_value = 0;
  analog_value = analogRead(ANALOG_PIN_0);
  delay(1000);
  Serial.printf("Current Reading on Pin(%d)=%d\n",ANALOG_PIN_0,analog_value);
  delay(3000);
}
