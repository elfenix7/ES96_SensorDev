#define led 10 // Put LED indicator output on pin 10
#define dial A0 // Put frequency sweeper input on pin A0
#define on 14 // Put on switch input on pin 14
#define SoundSensorPin A1 // Put sound sensor input on pin A1
#define button A2 // Put button input on pin 15
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

#include "SparkFun_AS7265X.h" //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;

#include <Wire.h> 

bool sense = 0;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(on, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  lcd.begin(16, 2);
  Serial.begin(115200);
  sound.prevDBtime = 0;

  if(sensor.begin() == false)
  {
    Serial.println("Sensor does not appear to be connected. Please check wiring. Freezing...");
    while(1);
  }
  else
    Serial.println("Good to go!");

  //Once the sensor is started we can increase the I2C speed
  Wire.setClock(400000);

  //Set Sensor Gain
  //There are four gain settings. It is possible to saturate the reading so don't simply jump to 64x.
  //sensor.setGain(AS7265X_GAIN_1X); //Default
  //sensor.setGain(AS7265X_GAIN_37X); //This is 3.7x
  sensor.setGain(AS7265X_GAIN_16X); 
  //sensor.setGain(AS7265X_GAIN_64X);

  //Set Measurement Mode (change _ONE_SHOT to _CONTINUOUS if needed)
  sensor.setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT); //Default: All 6 channels, all devices, just once per request

  //Integration cycles is from 0 (2.78ms) to 255 (711ms)
  sensor.setIntegrationCycles(49); //Default 50*2.8ms = 140ms per reading
  //sensor.setIntegrationCycles(1); //2*2.8ms = 5.6ms per reading

  // Set Drive current for illumination LEDs
  //4 levels: 12.5, 25, 50, and 100mA
  // White is max 120ma, UV max 30, IR max 65
  // These seem to give good illumination
  sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE); //Default
  sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV); //Default
  sensor.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR); //Default

  // Disabled indicator LED  after a blink to prevent over illumination in Blue spectrum
  sensor.enableIndicator();
  sensor.setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_8MA);
  // Enable interrupt commands
  sensor.enableInterrupt();
  // Test Print
  Serial.println("A,B,C,D,E,F,G,H,I,J,K,L,R,S,T,U,V,W");
  sensor.disableIndicator();
}

void loop() 
{
  if (digitalRead(on))  // Do sound sensing
  {
    soundsense();
    sense = 0;
  }
  else
  {
    opticalsense();
  }
}

void opticalsense() // do optical sensing
{
  if (sense == 0)
  {
    lcd.clear();
    lcd.print("optical sensing");
    sense = 1;
  }
  if (digitalRead(button) == LOW) // Start the test when ready
  {
    // Continuous Measurements
    sensor.takeMeasurementsWithBulb(); //This is a hard wait while all 18 channels are measured
    // Read and space with commas for CSV format, gaps are to indicate the three sensor packages
    Serial.print(sensor.getCalibratedA());
    Serial.print(",");
    Serial.print(sensor.getCalibratedB());
    Serial.print(",");
    Serial.print(sensor.getCalibratedC());
    Serial.print(",");
    Serial.print(sensor.getCalibratedD());
    Serial.print(",");
    Serial.print(sensor.getCalibratedE());
    Serial.print(",");
    Serial.print(sensor.getCalibratedF());
    Serial.print(",");
  
    Serial.print(sensor.getCalibratedG());
    Serial.print(",");
    Serial.print(sensor.getCalibratedH());
    Serial.print(",");
    Serial.print(sensor.getCalibratedI());
    Serial.print(",");
    Serial.print(sensor.getCalibratedJ());
    Serial.print(",");
    Serial.print(sensor.getCalibratedK());
    Serial.print(",");
    Serial.print(sensor.getCalibratedL());
    Serial.print(",");
  
    Serial.print(sensor.getCalibratedR());
    Serial.print(",");
    Serial.print(sensor.getCalibratedS());
    Serial.print(",");
    Serial.print(sensor.getCalibratedT());
    Serial.print(",");
    Serial.print(sensor.getCalibratedU());
    Serial.print(",");
    Serial.print(sensor.getCalibratedV());
    Serial.print(",");
    Serial.print(sensor.getCalibratedW());
    //Blank line for spacing
    Serial.println();
    // end of continuous measurements function
  
    // Prompted Measurements 
  }
}

void soundsense() // do sound sensing
{
  digitalWrite(led, LOW); // Turn the led off until starting test
  frequency = freq(frequency); // Read the frequency from the frequency sweeper
  sound = printscreen(sound.prevDBtime, millis()); // Keep reading values until ready
  if (digitalRead(button) == LOW) // Start the test when ready
  {
    tone(buzzer, frequency);
    delay(500);
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
    digitalWrite(led, HIGH);
    while (digitalRead(button) == HIGH) // Press the button to signal ready for attenuated sensing
    {
      // do nothing
    }
    delay(500);
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
    noTone(buzzer);
    digitalWrite(led, HIGH);
    Serial.print(base_avg - attn_avg);
    Serial.println(" dB atten");
    while (digitalRead(button) == HIGH) // Press the button to clear attenuated value
    {
      // do nothing
    }
    delay(500);
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
