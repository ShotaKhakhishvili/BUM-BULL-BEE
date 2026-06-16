#pragma once

// One motor channel on a TB6612FNG: a PWM (speed) pin plus two
// direction pins (IN1/IN2).
class Wheel
{
private:
  bool dir = false;
  int pwm, in1, in2;

public:

  Wheel(int pwmIn, int in1In, int in2In);

  void Init();                          // set pin modes
  void setRotation(bool dir, int str);
};
