void setup() {
  Serial.begin(115200);
}

void loop() {
  // Pull out A0 and connect it to different Regions of the circuit:
  // 1. Positive strip to see if its close to 3.3V or 5V
  // 2. Jnnction between the resistors 
  // 3. Junction with mic pin0 + resistor
  // 4. Ground = 0.0V
  int raw = analogRead(A0);
  float voltage = raw * (3.3 / 1023.0);
  
  Serial.print("A0 Raw: ");
  Serial.print(raw);
  Serial.print(" = ");
  Serial.print(voltage);
  Serial.println("V");
  Serial.print("Signal: ");
  Serial.println(raw);
  delay(500);
}