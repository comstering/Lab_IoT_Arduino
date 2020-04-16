/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
 outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.
 
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Use this calibration_factor on the example sketch
 
 This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
 calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).
 
 Your calibration factor may be very positive or very negative. It all depends on the setup of your scale system
 and the direction the sensors deflect from zero state
 This example code uses bogde's excellent library: https://github.com/bogde/HX711
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE
 Arduino pin 2 -> HX711 CLK
 3 -> DOUT
 5V -> VCC
 GND -> GND
 
 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.
 
 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
 
*/
 
//This library can be obtained here http://librarymanager/All#Avia_HX711
#include "HX711.h" 
 
#define LOADCELL_DOUT_PIN D5
#define LOADCELL_SCK_PIN D6
 
HX711 scale;

float output = 0;
 
//-7050 worked for my 440lb max scale setup
float calibration_factor = 20000;
 
void setup() 
{
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  //Reset the scale to 0
  scale.tare();
 
  //Get a baseline reading
  long zero_factor = scale.read_average(); 
  //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
}
 
void loop() 
{
  //Adjust to this calibration factor
  scale.set_scale(calibration_factor);

  output = scale.get_units(), 1;
  output /= 10;
  Serial.print("Reading: ");
  Serial.print(output);
  //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  //기존 예제가 파운드(lbs) 기준이지만 우리는 킬로그램(kg)을 쓸것이므로 'kg'로 바꿉시다.
  Serial.print(" kg"); 
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();
 
  if(Serial.available())
  {
    char temp = Serial.read();
 
    //변경 : 보정값 범위 설정 가능하도록 변경
    switch(temp)
    {
      case '1':
        calibration_factor += 10;
        break;
      case '2':
        calibration_factor += 50;
        break;
      case '3':
        calibration_factor += 100;
        break;
      case '4':
        calibration_factor += 1000;
        break;
 
      case 'a':
        calibration_factor -= 10;
        break;
      case 's':
        calibration_factor -= 50;
        break;
      case 'd':
        calibration_factor -= 100;
        break;
      case 'f':
        calibration_factor -= 1000;
        break;
    }
  }
  delay(1000);
}
