#define led 10 // Put LED indicator output on pin 10
#define dial A0 // Put frequency sweeper input on pin A0
#define on 14 // Put on switch input on pin 14
#define SoundSensorPin A1 // Put sound sensor input on pin A1
#define button 15 // Put button input on pin 15
#define VREF  5.0 // Arduion voltage

int buzzer = 16; // Put buzzer output on pin 16
int ledState = LOW; // Start the LED off

float frequency = 100; // Starting frequency

unsigned long prevLEDtime = 0;
unsigned long currentMillis = 0;

struct soundpackage
{
  unsigned long prevDBtime;
  float soundlevel;
};
soundpackage sound;

#include <LiquidCrystal.h>
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(on, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  lcd.begin(16, 2);
  Serial.begin(115200);
  sound.prevDBtime = 0;
}

void loop() 
{
  if (digitalRead(on))  // Turn on sound if on switch is on
    tone(buzzer, frequency);
  else
    noTone(buzzer);
    
  digitalWrite(led, LOW); // Turn the led off until starting test
  frequency = freq(frequency); // Read the frequency from the frequency sweeper
  if (digitalRead(button) == LOW) // Start the test after 1 second delay
  {
    digitalWrite(led, HIGH);
    delay(1000);
    float base_avg = 0;
    int base_ctr = 0;
    float attn_avg = 0;
    int attn_ctr = 0;
    currentMillis = millis(); // Get the time
    while (millis() - currentMillis < 5000) // Get background sound level for 5 seconds
    {

      sound = printscreen(sound.prevDBtime, millis()); // Print values on LCD screen
      base_avg += sound.soundlevel;
      base_ctr += 1;
      prevLEDtime = LED(prevLEDtime, millis(), 500); // Flash LED twice a second
    }
    base_avg = base_avg/base_ctr; // Calculate average base sound levels
    digitalWrite(led, HIGH); // Wait for 3 seconds, then back at it
    delay(3000);
    currentMillis = millis(); // Get the time
    while (millis() - currentMillis < 5000) // Get the attenuated sound level for 5 seconds
    {
      sound = printscreen(sound.prevDBtime, millis()); // Print values on LCD screen
      attn_avg += sound.soundlevel;
      attn_ctr += 1;
      prevLEDtime = LED(prevLEDtime, millis(), 250); // Flash LED four times a second      
    }
    attn_avg = attn_avg/attn_ctr; // Calculate average attenuated sound levels
    lcd.clear(); // Print attenuation and end test
    lcd.print((int) frequency);
    lcd.setCursor(0,1);
    lcd.print(base_avg - attn_avg);
    lcd.setCursor(5,0);
    lcd.print(" Hz");
    lcd.setCursor(5,1);
    lcd.print(" dB atten.");
    digitalWrite(led, LOW);
  }
}

float freq(float f) // Return a new fequency based on scrolling the frequency sweeper
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

soundpackage printscreen(unsigned long prevDBtime, unsigned long currentMillis) // Print sound level and frequency if .5 seconds have passed
{
  lcd.setCursor(5,0);
  lcd.print(" Hz");
  lcd.setCursor(5,1);
  lcd.print(" dB");
  if (currentMillis - prevDBtime >= 500)
  {
    sound.prevDBtime = currentMillis;
    float voltageValue,dbValue;
    voltageValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
    dbValue = voltageValue * 50.0;  //convert voltage to decibel value
    sound.soundlevel = dbValue;
    Serial.print(dbValue,1);
    Serial.println(" dBA");
    lcd.clear();
    lcd.print((int) frequency);
    lcd.setCursor(0,1);
    lcd.print(dbValue); 
  }
  return sound;
}

unsigned long LED(unsigned long prevLEDtime, unsigned long currentMillis, int ledTimer) // Flash the LED accordingly
{
  if (currentMillis - prevLEDtime >= ledTimer) // LED indicator code
  {
    prevLEDtime = currentMillis;
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    if (digitalRead(on))
      digitalWrite(led, ledState);
    else
      digitalWrite(led, LOW);
  }
  return prevLEDtime;
}
