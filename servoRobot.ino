#include <Servo.h>
#include <Logging.h>
#include "servoHndl.h"

#define BUTTON_1_PIN 10
#define BUTTON_2_PIN 16
#define BTN_BLACK BUTTON_1_PIN
#define BTN_RED   BUTTON_2_PIN

const byte numChars = 64;
char receivedChars[numChars];
char strNewValue[4];
char strServoNum[2];

#define COMMAND_SEPARATOR ';'

#define USE_RELATIVE_VALUES 1
#define USE_ABSOLUTE_VALUES 2
char servoMovement = USE_RELATIVE_VALUES;

boolean newData = false;

unsigned long currentMillis  = millis();
unsigned long previousMillis = 0;

void setup() 
{
  short i = 0; 
  //Serial.begin(115200);   --> this is called in the logger 
  Log.Init(LOGLEVEL, 115200L, AUTO_NEWLINE_ON);
  Log.Disable();
  /* SET THE BUTTON PINS */
  pinMode     (BTN_RED, INPUT_PULLUP);
  digitalWrite(BTN_RED, HIGH);
  pinMode     (BTN_BLACK, INPUT_PULLUP);
  digitalWrite(BTN_BLACK, HIGH);
  /* INITIALIZE THE SERVO MOTORS */
  for( i = 0; i < NSERVOS; i++)
  {
    /* SERVO SPECIFIC SETTINGS          */
    switch(i)
    {
      case 0:
        servosH[i].servoPin = SERVO_PIN_0;
      break;

      case 1:
        servosH[i].servoPin = SERVO_PIN_1;
      break;

      case 2:
        servosH[i].servoPin = SERVO_PIN_2;
      break;

      case 3:
        servosH[i].servoPin = SERVO_PIN_3;
      break;
    }

    /* COMMON SETTINGS                        */
    servosH[i].isMoving = false;
    servosH[i].microMIN = 600;
    servosH[i].microMAX = 2500;
    servosH[i].angleMIN = 0;
    servosH[i].angleMAX = 180;
    servosH[i].movementDelay =  1;
    servosH[i].movementStep = 5;
    servosH[i].servo.attach(servosH[i].servoPin);
    servosH[i].currentAngle = SERVO_START_ANGLE;
    mapAngleToMicroSeconds(i);
  }

  resetServosToInitialPosition();
}

void resetServosToInitialPosition()
{
  for( int i = 0; i < NSERVOS; i++)
    moveServo(SERVO_START_ANGLE, i, USE_ABSOLUTE_VALUES);
}

void loop() 
{  
  handleButtonEvents();
  currentMillis  = millis();
  if(currentMillis - previousMillis > 25)
  {
    previousMillis = currentMillis;
    serialRecvCmd(COMMAND_SEPARATOR);
    serialLogRcvData();
    if (newData == true)
    {
      moveServo( atoi(strNewValue), atoi(strServoNum), servoMovement);
      newData = false;
    }
  }
}

float mapAngleToMicroSeconds(const int servoNumber)
{
  servosH[servoNumber].microMove = map(servosH[servoNumber].currentAngle, 
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


void moveServo(const int servoAngle, const short servoNumber, const char servoMovement)
{
  int  newAngle = 0;

  /* Current servo is currently moving     */
  if(servosH[servoNumber].isMoving == true)
  {
    Log.Info("Servo %d is moving. Request to move it to %d dropped [%d]", servoNumber, servoAngle, servoMovement);
    return;
  }

  if( servoMovement == USE_ABSOLUTE_VALUES )
        newAngle = servoAngle;
  else if( servoMovement == USE_RELATIVE_VALUES )
        newAngle = servosH[servoNumber].currentAngle + servoAngle;

  /* IGNORE a move which doesn't do anythin */
  if(servosH[servoNumber].currentAngle == newAngle)
    return;

  Log.Info("[%s] Moving servo %d from %d to %d",
          ( (servoMovement == USE_ABSOLUTE_VALUES) ? "A" : "R" ),
          servoNumber, 
          servosH[servoNumber].currentAngle, 
          newAngle);
  
  /* CHECK FOR INVALID MOVES                */
  if (servosH[servoNumber].angleMIN <= newAngle && newAngle <= servosH[servoNumber].angleMAX)
  {
    int limit         = newAngle + ( (newAngle < servosH[servoNumber].currentAngle) ? (-1) : 1);
    int stepDirection = ( newAngle < servosH[servoNumber].currentAngle) ? (-1) : 1;
    int step          = ( (stepDirection*(newAngle - servosH[servoNumber].currentAngle)) < servosH[servoNumber].movementStep) ? 1 : servosH[servoNumber].movementStep;
    
    servosH[servoNumber].isMoving = true;
      for (; servosH[servoNumber].currentAngle != limit; 
             servosH[servoNumber].currentAngle += step * stepDirection)
      {
        Log.Info("    > currentAngle = %d", servosH[servoNumber].currentAngle);
        mapAngleToMicroSeconds(servoNumber);
        microMoveServo(servoNumber);
        if(servosH[servoNumber].movementDelay)
          delay(servosH[servoNumber].movementDelay);
         step          = ( (stepDirection*(newAngle - servosH[servoNumber].currentAngle)) < servosH[servoNumber].movementStep) ? 1 : servosH[servoNumber].movementStep;
      }

    servosH[servoNumber].currentAngle = newAngle;
    servosH[servoNumber].isMoving = false;
  }
  else
    Log.Error("Angle received [%d] is outside motor %d range [%d - %d]",
              newAngle, servoNumber, 
              servosH[servoNumber].angleMIN, 
              servosH[servoNumber].angleMAX);
}

void serialFlush()
{
  Log.Debug(" >> Flushing serial input <<");
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
      Log.Error(" > Invalid command received [%d]", rc);
      serialFlush();
      break;
    }
    commandValidated = true;
    if(rc == USE_ABSOLUTE_VALUES)
      servoMovement = USE_ABSOLUTE_VALUES;

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
    Log.Info(" RCV Data: %s ===> %s", strNewValue, strServoNum);
    if( atoi(strServoNum) >= NSERVOS)
    {
      Log.Error("Invalid servo number received.");
      newData = false;
    }
  }
}

void onBlackButtonPress(int buttonState)
{
  if( buttonState == LOW )
  {
    Log.Debug(" > BLACK BUTTON PRESSED");
    resetServosToInitialPosition();
  }
}

void onRedButtonPress(int buttonState)
{
  if( buttonState == LOW )
  {
    #define SRV_0 0
    #define SRV_1 1
    #define SRV_2 2 
    ServoHndl s0 = servosH[SRV_0];
    ServoHndl s1 = servosH[SRV_1];
    ServoHndl s2 = servosH[SRV_2];

    static int delta0 =  10;
    static int delta1 =  10;
    static int delta2 =  10;

    Log.Debug(" > RED BUTTON PRESSED");

    /*************************************************************************************/
    /* MOVE SERVO 0                                                                      */
    if(s0.currentAngle > 170 || s0.currentAngle < 10)
    {
        delta0 *= -1;
        Log.Debug(" >>> [%d] - delta:%d; angle: %d;", SRV_0, delta0, s0.currentAngle);
    }
    moveServo(delta0, SRV_0, USE_RELATIVE_VALUES);
    /*************************************************************************************/
    /* MOVE SERVO 1                                                                      */
    if(s1.currentAngle > 170 || s1.currentAngle < 10)
    {
        delta1 *= -1;
        Log.Debug(" >>> [%d] - delta:%d; angle: %d;", SRV_1, delta1, s1.currentAngle);
    }
    moveServo(delta1, SRV_1, USE_RELATIVE_VALUES);
    /*************************************************************************************/
    /* MOVE SERVO 2                                                                      */
    if(s2.currentAngle > 170 || s2.currentAngle < 10)
    {
        delta2 *= -1;
        Log.Debug(" >>> [%d] - delta:%d; angle: %d;", SRV_2, delta0, s0.currentAngle);
    }
    moveServo(delta2, SRV_2, USE_RELATIVE_VALUES);
  }
}

void handleButtonEvents()
{
  onRedButtonPress  ( digitalRead(BTN_RED   ) );
  onBlackButtonPress( digitalRead(BTN_BLACK ) );
}