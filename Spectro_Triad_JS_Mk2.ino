/*
 * Read in Data code for Spectral Triad Sensor
 * By Joseph Sanchez
 * Adapted from Sparkfun Spectral Triad Library Examples by Nathan Seidle
 * 
 * Serial Monitor at 9600 Baud
 * Spectral Triad attached via I2C pinout or QWIIC cable
 */

#include "SparkFun_AS7265X.h" //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;

#include <Wire.h> 

void setup() {
  Serial.begin(9600); // Start Serial Communications
  Serial.println("AS7265x Spectral Triad Example");

  if(sensor.begin() == false)
  {
    Serial.println("Sensor does not appear to be connected. Please check wiring. Freezing...");
    while(1);
  }

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

void loop() {
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
  Serial.print(",");
  //Blank line for spacing
  Serial.println();
  // end of continuous measurements function

  // Prompted Measurements
  

}
