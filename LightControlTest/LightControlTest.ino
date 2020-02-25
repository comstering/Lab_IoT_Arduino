#include <Servo.h>

static const int servoPin = D6;

Servo servo;

char dir; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  servo.attach(servoPin);
  
  servo.write(90);
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(1500);
  if(Serial.available())
  {
    dir = Serial.read();
    if(dir == '1')
    {
      servo.write(60);
      delay(1000);
      servo.write(90);
    }
    else if(dir == '2')
    {
      servo.write(135);
      delay(1000);
      servo.write(90);
    }
  }

}
