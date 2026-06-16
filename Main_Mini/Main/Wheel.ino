#include "Wheel.hpp"

Wheel::Wheel(int pwmIn, int in1In, int in2In)
: pwm(pwmIn), in1(in1In), in2(in2In) {}

void Wheel::Init()
{
  pinMode(pwm, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void Wheel::setRotation(bool dir, int str)
{
  if(dir != this->dir)
  {
    // brief brake/coast on direction change (mirrors the old behavior)
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(pwm, 0);
    delay(1);
  }
  if(dir)
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  analogWrite(pwm, str);
  this->dir = dir;

  Serial.print("Motor (PWM pin ");
  Serial.print(pwm);
  Serial.print(") | Dir: ");
  Serial.print(dir ? "FWD" : "BWD");
  Serial.print(" | Speed: ");
  Serial.println(str);
}
