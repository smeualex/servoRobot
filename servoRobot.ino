#include <Servo.h>

#define BUTTON_1_PIN 10
#define BUTTON_2_PIN 16

#define BTN_RED   BUTTON_1_PIN
#define BTN_BLACK BUTTON_2_PIN

const byte numChars = 64;
char receivedChars[numChars];
char strNewValue[4];
char strServoNum[2];


#define USE_RELATIVE_VALUES 1
#define USE_ABSOLUTE_VALUES 2
char SERVO_MOVEMENT = USE_RELATIVE_VALUES;

boolean newData = false;

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

unsigned long currentMillis  = millis();
unsigned long previousMillis = 0;

void setup() 
{
  short i = 0; 
  Serial.begin(9600);
  
  /* SET THE BUTTON PINS */
  pinMode     (BTN_RED, INPUT_PULLUP);
  digitalWrite(BTN_RED, HIGH);

  pinMode     (BTN_BLACK, INPUT_PULLUP);
  digitalWrite(BTN_BLACK, HIGH);

  /* INITIALIZE THE SERVO MOTORS */
  for( i = 0; i < NSERVOS; i++)
  {
    switch(i)
    {
      case 0:
        servosH[i].servoPin = SERVO_PIN_0;
        servosH[i].microMIN = 600;
        servosH[i].microMAX = 2500;
        servosH[i].angleMIN = 0;
        servosH[i].angleMAX = 180;
        servosH[i].movementDelay =  1;
        servosH[i].movementStep = 1;
      break;

      case 1:
        servosH[i].servoPin = SERVO_PIN_1;
        servosH[i].microMIN = 600;
        servosH[i].microMAX = 2500;
        servosH[i].angleMIN = 0;
        servosH[i].angleMAX = 180;
        servosH[i].movementDelay =  1;
        servosH[i].movementStep = 1;
      break;

      case 2:
        servosH[i].servoPin = SERVO_PIN_2;
        servosH[i].microMIN = 600;
        servosH[i].microMAX = 2500;
        servosH[i].angleMIN = 0;
        servosH[i].angleMAX = 180;
        servosH[i].movementDelay =  1;
        servosH[i].movementStep = 1;
      break;

      case 3:
        servosH[i].servoPin = SERVO_PIN_3;
        servosH[i].microMIN = 600;
        servosH[i].microMAX = 2500;
        servosH[i].angleMIN = 0;
        servosH[i].angleMAX = 180;
        servosH[i].movementDelay =  1;
        servosH[i].movementStep = 1;
      break;
    }

    servosH[i].servo.attach(servosH[i].servoPin);
    servosH[i].currentAngle = SERVO_START_ANGLE;
    mapAngleToMicroSeconds(i, SERVO_START_ANGLE);
  }

  resetServosToInitialPosition();
}

void resetServosToInitialPosition()
{
  for( int i = 0; i < NSERVOS; i++)
    moveServo(servosH[i].currentAngle, i);
}

void loop() 
{  
  checkButtons();

  currentMillis  = millis();
  if(currentMillis - previousMillis > 25)
  {
    previousMillis = currentMillis;
    serialRecvCmd(';');
    serialLogRcvData();
    
    if (newData == true)
    {
      int newValue    = atoi(strNewValue);
      int servoNumber = atoi(strServoNum);
      
      if( SERVO_MOVEMENT == USE_ABSOLUTE_VALUES )
        moveServo(newValue, servoNumber);
      else if( SERVO_MOVEMENT == USE_RELATIVE_VALUES )
        moveServo(servosH[servoNumber].currentAngle + newValue, servoNumber);

      newData = false;
    }
  }
  
}

float mapAngleToMicroSeconds(const int angle, const int servoNumber)
{
  servosH[servoNumber].microMove = map(angle, 
                                      servosH[servoNumber].angleMIN, 
                                      servosH[servoNumber].angleMAX, 
                                      servosH[servoNumber].microMIN, 
                                      servosH[servoNumber].microMAX);

  return servosH[servoNumber].microMove;
}

void microMoveServo( const short servoNumber )
{
  servosH[servoNumber].servo.writeMicroseconds(servosH[servoNumber].microMove);
}


void moveServo(const int newAngle, const short servoNumber)
{
  char logMsg[128] = {0, };

  if(servosH[servoNumber].currentAngle == newAngle)
    return;

  sprintf(logMsg, "Moving servo %d from %d to %d", servoNumber, servosH[servoNumber].currentAngle, newAngle);
  Serial.println(logMsg);
  if (newAngle >= 0 && newAngle <= 180)
  {
    if (newAngle < servosH[servoNumber].currentAngle)
      for (; servosH[servoNumber].currentAngle > newAngle; servosH[servoNumber].currentAngle -= servosH[servoNumber].movementStep)
      {
        servosH[servoNumber].servo.writeMicroseconds(mapAngleToMicroSeconds(servosH[servoNumber].currentAngle, servoNumber));
        delay(servosH[servoNumber].movementDelay);
      }
    else
      for (; servosH[servoNumber].currentAngle < newAngle; servosH[servoNumber].currentAngle += servosH[servoNumber].movementStep)
      {
        servosH[servoNumber].servo.writeMicroseconds(mapAngleToMicroSeconds(servosH[servoNumber].currentAngle, servoNumber));
        delay(servosH[servoNumber].movementDelay);
      }
    servosH[servoNumber].currentAngle = newAngle;
  }
  else
    Serial.println("Invalid angle received");

}

void serialFlush()
{
  Serial.println(" >> Flushing serial input <<");
  while(Serial.available())
    Serial.read();
}

// recv command from serial comm with following format:
//    CTYPEVAL#NUMSERVO;
//      CTYPE     - command type: Relative or Absolute position - 1 or 2
//      VAL       - new angle to move to                        - +/- -180:180
//      NUMSERVO  - the servo to move                           - 0:3
void serialRecvCmd(char endMarker) 
{
  bool   commandValidated = false;
  bool   angleReceived    = false;
  static byte ndx = 0;
  char   rc;

  // if there is a not process data -> flush the incoming serial data
  if( newData == true )
    serialFlush();

  while (Serial.available() > 0 && newData == false) 
  {
    rc = Serial.read();

    if( commandValidated == false 
      && rc != (char)(USE_RELATIVE_VALUES + '0') 
      && rc != (char)(USE_ABSOLUTE_VALUES + '0'))
    {
      Serial.print(" > Invalid command received ");
      Serial.println(rc);
      serialFlush();
      break;
    }
    commandValidated = true;

    if (rc != endMarker) 
    {
      receivedChars[ndx] = rc;
      ndx++;

      if( angleReceived == false && rc == '#' )
      {
        angleReceived = true;
        strncpy(strNewValue, &receivedChars[1], ndx-1);
        strNewValue[ndx-1] = 0;
        memset(receivedChars, 0, sizeof(receivedChars));
        ndx = 0;
      }
      
      if (ndx >= numChars) 
        ndx = numChars - 1;
    }
    else 
    {
      receivedChars[ndx] = '\0'; // terminate the string
      strcpy(strServoNum, receivedChars);
      ndx = 0;
      newData = true;
    }
  }
}

void serialLogRcvData() 
{
  if (newData == true) 
  {
    Serial.print("This just in ... ");
    Serial.print(strNewValue);
    Serial.print(" ==> ");
    Serial.println(strServoNum);
    if( atoi(strServoNum) >= NSERVOS)
    {
      Serial.print("Invalid servo number received. ");
      newData = false;
    }
  }
}

void checkButtons()
{
  int redButtonState   = digitalRead(BTN_RED);
  int blackButtonState = digitalRead(BTN_BLACK);
  
  ServoHndl s1 = servosH[0];
  ServoHndl s2 = servosH[1];

  static int delta1 = 10;
  static int delta2 = 10;

  if( redButtonState == LOW )
  {
    Serial.println(" > RED BUTTON PRESSED");
    if(s1.currentAngle > 170 || s1.currentAngle < 10)
    {
        delta1 *= -1;
        Serial.print(" >>> delta1: "); Serial.print(delta1); Serial.print("; Servo 0 angle: "); Serial.println(s1.currentAngle);
    }
    moveServo(s1.currentAngle+delta1, 0);
    delay(10);
  }

  if( blackButtonState == LOW )
  {
    Serial.println(" > BLACK BUTTON PRESSED");
    if(s2.currentAngle > 170 || s2.currentAngle < 10)
    {
        delta2 *= -1;
        Serial.print(" >>> delta2: "); Serial.print(delta2); Serial.print("; Servo 1 angle: "); Serial.println(s2.currentAngle);
    }
    moveServo(s2.currentAngle+delta2, 1);
    delay(10);
  }

}