#define led 10
#define dial A0
#define on 14
#define SoundSensorPin A1
#define VREF  5.0
int buzzer = 16;
float frequency = 100;
int ledState = LOW;
unsigned long previousMillis = 0;
unsigned long currentMillis  = 0;
unsigned long prevDB = 0;
#include <LiquidCrystal.h>
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(on, INPUT_PULLUP);
  lcd.begin(16, 2);
  Serial.begin(115200);
}

void loop() 
{
  frequency = freq(frequency);
  if (digitalRead(on))
    tone(buzzer, frequency);
  else
    noTone(buzzer);

  lcd.setCursor(5,0);
  lcd.print(" Hz");
  lcd.setCursor(5,1);
  lcd.print(" dB");
  currentMillis = millis();
  if (currentMillis - prevDB >= 500)
  {
    prevDB = currentMillis;
    float voltageValue,dbValue;
    voltageValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
    dbValue = voltageValue * 50.0;  //convert voltage to decibel value
    Serial.print(dbValue,1);
    Serial.println(" dBA");
    lcd.clear();
    lcd.print((int) frequency);
    lcd.setCursor(0,1);
    lcd.print(dbValue); 
  }
  if (currentMillis - previousMillis >= map(analogRead(dial),0,1024,1024,0))
  {
    previousMillis = currentMillis;
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    if (digitalRead(on))
      digitalWrite(led, ledState);
    else
      digitalWrite(led, LOW);
  }
}

float freq(float f)
{
  int dialvalue = analogRead(dial);
  if (dialvalue < 400)
    f -= exp((float) map(dialvalue,399,0,-4,5))/(10^5);
  if (dialvalue > 624)
    f += exp((float) map(dialvalue,625,1024,-4,5))/(10^5);
  if (f > 13000)
    f = 13000;
  if (f < 40)
    f = 40;
  return f;
}
