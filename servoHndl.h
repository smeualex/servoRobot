#define SERVO_PIN_0 6;
#define SERVO_PIN_1 7;
#define SERVO_PIN_2 8;
#define SERVO_PIN_3 9;

#define SERVO_START_ANGLE 90

#define NSERVOS 4
struct ServoHndl
{
#define MAX_HIST_STEPS 25
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
//  short angleHistory[MAX_HIST_STEPS];
} servosH[NSERVOS];