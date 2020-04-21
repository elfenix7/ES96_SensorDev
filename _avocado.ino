#define led 10 // Put LED indicator output on pin 10
#define dial A0 // Put frequency sweeper input on pin A0
#define switcher 14 // Put on switch input on pin 14
#define SoundSensorPin A1 // Put sound sensor input on pin A1
#define button A2 // Put button input on pin 15
#define VREF  5.0 // Arduion voltage

int buzzer = 16; // Put buzzer output on pin 16
int ledState = LOW; // Start the LED off

const int frequency = 4700; // Starting frequency
int ident = 0; // Starting identifier location
int ident_old = 0; // For detecting if identifier location was just changed
int stage_old = 1; // For detecting if stage number was just changed

String names_list[41] = {
  "mh1",
  "mh2",
  "mh3",
  "mh4",
  "mh5",
  "mh6",
  "mh7",
  "mh8",
  "mh9",
  "mh10",
  "mc1",
  "mc2",
  "mc3",
  "mc4",
  "mc5",
  "mc6",
  "mc7",
  "mc8",
  "mc9",
  "mc10",
  "mr1",
  "mr2",
  "mr3",
  "mr4",
  "mr5",
  "mr6",
  "mr7",
  "mr8",
  "mr9",
  "mr10",
  "mr11",
  "mr12",
  "mr13",
  "mr14",
  "mr15",
  "mr16",
  "mr17",
  "mr18",
  "mr19",
  "mr20"
};

unsigned long prevLEDtime = 0; // For LED timing purposes
unsigned long currentMillis = 0; // For general timing purposes
unsigned long prevNametime = 0; // For identifier update timing purposes
unsigned long prevStagetime = 0; // For stage update timing purposes

bool sense = 0; // For mode switching purposes
int sense_switch = 0; // For mode labeling
int sense_switched = 0; // For detecting if a mode was just switched

struct soundpackage // Used in the sound printscreen function
{
  unsigned long prevDBtime; // For sound timing purposes
  float soundlevel; // Records the sound level
};
soundpackage sound;

struct serialrecord // What the computer reads via serial
{
  String identifier = "mh1";
  int stage = 1;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  float G;
  float H;
  float I;
  float J;
  float K;
  float L;
  float R;
  float S;
  float T;
  float U;
  float V;
  float W;
  float dB;
};
serialrecord serial;

#include <LiquidCrystal.h>
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
#include "SparkFun_AS7265X.h" //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;
#include <Wire.h> 

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(switcher, INPUT_PULLUP);
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
  if (digitalRead(switcher) != sense) // This detects when you want to switch modes
  {
    sense_switch += 1;
    if (sense_switch > 2)
      sense_switch = 0;
    sense = digitalRead(switcher);
  }
  switch (sense_switch)
  {
    case 0:
      soundsense(); // Sound sensing mode
      sense_switched = sense_switch;
      break;
    case 1:
      opticalsense(); // Optical sensing mode
      sense_switched = sense_switch;
      break;
    case 2:
      serialsend(); // Serial sending mode
      sense_switched = sense_switch;
      break;
  }
}

void soundsense() // do sound sensing
{
  digitalWrite(led, LOW); // Turn the led off until starting test
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
    while (millis() - currentMillis < 5000) // Get attenuated sound level for 5 seconds
    {

      sound = printscreen(sound.prevDBtime, millis()); // Print values on LCD screen
      attn_avg += sound.soundlevel;
      attn_ctr += 1;
      prevLEDtime = LED(prevLEDtime, millis(), 500); // Flash LED twice a second
    }
    attn_avg = attn_avg/attn_ctr; // Calculate average attenuated sound levels
    digitalWrite(led, HIGH);
    while (digitalRead(button) == HIGH) // Press the button to signal ready for background sensing
    {
      // do nothing
    }
    delay(500);
    currentMillis = millis(); // Get the time
    while (millis() - currentMillis < 5000) // Get the background sound level for 5 seconds
    {
      sound = printscreen(sound.prevDBtime, millis()); // Print values on LCD screen
      base_avg += sound.soundlevel;
      base_ctr += 1;
      prevLEDtime = LED(prevLEDtime, millis(), 250); // Flash LED four times a second      
    }
    base_avg = base_avg/base_ctr; // Calculate average background sound levels
    lcd.clear(); // Print attenuation and end test
    lcd.print(frequency);
    lcd.setCursor(0,1);
    lcd.print(base_avg - attn_avg);
    lcd.setCursor(5,0);
    lcd.print(" Hz");
    lcd.setCursor(5,1);
    lcd.print(" dB atten.");
    noTone(buzzer);
    digitalWrite(led, HIGH);
    serial.dB = base_avg - attn_avg;
    while (digitalRead(button) == HIGH) // Press the button to clear attenuated value
    {
      // do nothing
    }
    delay(500);
    digitalWrite(led, LOW);
  }  
}

void opticalsense() // do optical sensing
{
  if (sense_switched != sense_switch)
  {
    lcd.clear();
    lcd.print("optical sensing");
  }
  if (digitalRead(button) == LOW) // Start the test when ready
  {
    // Continuous Measurements
    sensor.takeMeasurementsWithBulb(); //This is a hard wait while all 18 channels are measured
    // Read and space with commas for CSV format, gaps are to indicate the three sensor packages
    serial.A = sensor.getCalibratedA();
    serial.B = sensor.getCalibratedB();
    serial.C = sensor.getCalibratedC();
    serial.D = sensor.getCalibratedD();
    serial.E = sensor.getCalibratedE();
    serial.F = sensor.getCalibratedF();
  
    serial.G = sensor.getCalibratedG();
    serial.H = sensor.getCalibratedH();
    serial.I = sensor.getCalibratedI();
    serial.J = sensor.getCalibratedJ();
    serial.K = sensor.getCalibratedK();
    serial.L = sensor.getCalibratedL();
  
    serial.R = sensor.getCalibratedR();
    serial.S = sensor.getCalibratedS();
    serial.T = sensor.getCalibratedT();
    serial.U = sensor.getCalibratedU();
    serial.V = sensor.getCalibratedV();
    serial.W = sensor.getCalibratedW();
    // end of continuous measurements function
  }
}

void serialsend() // Sends the data via serial
{
  if (sense_switched != sense_switch)
  {
    lcd.clear();
    lcd.print("sending serial");
    lcd.setCursor(0,1);
    lcd.print(serial.identifier);
    lcd.print(" ");
    lcd.setCursor(5,1);
    lcd.print("stage: ");
    lcd.print(serial.stage);
  }
  while (digitalRead(button) == HIGH)
  {
    digitalWrite(led,HIGH);
    if (digitalRead(switcher) != sense)
    break;
    if (millis() - prevNametime >= 400)
    {
      ident = naming(ident);
      prevNametime = millis();  
    }
    serial.identifier = names_list[ident];
    if (ident != ident_old)
    {
      lcd.setCursor(0,1);
      lcd.print(serial.identifier);
      lcd.print(" ");
      ident_old = ident;
    }
  }
  delay(200);
  while (digitalRead(button) == HIGH)
  {
    digitalWrite(led,LOW);
    if (digitalRead(switcher) != sense)
    break;
    if (millis() - prevStagetime >= 400)
    {
      serial.stage = staging(serial.stage);
      prevStagetime = millis();  
    }
    if (serial.stage != stage_old)
    {
      lcd.setCursor(5,1);
      lcd.print("stage: ");
      lcd.print(serial.stage);
      stage_old = serial.stage;
    }  
  }
  // Send the serial data when ready
  if (digitalRead(button) == LOW)
  {
    Serial.print(serial.identifier);
    Serial.print(",");
    Serial.print(serial.stage);
    Serial.print(",");
    Serial.print(serial.A);
    Serial.print(",");
    Serial.print(serial.B);
    Serial.print(",");
    Serial.print(serial.C);
    Serial.print(",");
    Serial.print(serial.D);
    Serial.print(",");
    Serial.print(serial.E);
    Serial.print(",");
    Serial.print(serial.F);
    Serial.print(",");
    Serial.print(serial.G);
    Serial.print(",");
    Serial.print(serial.H);
    Serial.print(",");
    Serial.print(serial.I);
    Serial.print(",");
    Serial.print(serial.J);
    Serial.print(",");
    Serial.print(serial.K);
    Serial.print(",");
    Serial.print(serial.L);
    Serial.print(",");
    Serial.print(serial.R);
    Serial.print(",");
    Serial.print(serial.S);
    Serial.print(",");
    Serial.print(serial.T);
    Serial.print(",");
    Serial.print(serial.U);
    Serial.print(",");
    Serial.print(serial.V);
    Serial.print(",");
    Serial.print(serial.W);
    Serial.print(",");
    Serial.print(serial.dB);
    Serial.print(" dB");
    Serial.println();
    delay(500);
  }
}

int naming(int f) // Return a new identifier based on scrolling the name sweeper
{
  int dialvalue = analogRead(dial);
  if (dialvalue < 400)
    f -= map(dialvalue,399,0,1,5);
  if (dialvalue > 624)
    f += map(dialvalue,625,1024,1,5);
  if (f > 39)
    f = 0;
  if (f < 0)
    f = 39;
  return f;
}

int staging(int f) // Return a new stage based on scrolling the stage sweeper
{
  int dialvalue = analogRead(dial);
  if (dialvalue < 400)
    f -= map(dialvalue,399,0,1,5);
  if (dialvalue > 624)
    f += map(dialvalue,625,1024,1,5);
  if (f > 5)
    f = 5;
  if (f < 1)
    f = 1;
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
    lcd.print(frequency);
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
    digitalWrite(led, ledState);
  }
  return prevLEDtime;
}
