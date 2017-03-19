/**
* ARDUINO PINS
*/
#define SERVO_PIN_0 6;
#define SERVO_PIN_1 7;
#define SERVO_PIN_2 8;
#define SERVO_PIN_3 9;

#define NSERVOS 4
#define SERVO_START_ANGLE 90
struct ServoHndl
{
  Servo servo;
  bool  isMoving;
  int   servoPin;
  short currentAngle;
  
  float microMove;
  float microMIN;
  float microMAX;
  int   angleMIN;
  int   angleMAX;

  short movementDelay;
  short movementStep;
} servosH[NSERVOS];