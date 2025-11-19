#include <Servo.h>

#define leftmotor1 7
#define leftmotor2 6
#define leftena 5
#define rightena 3
#define rightmotor1 4
#define rightmotor2 2

#define servopin 9

//sensor
const int sensorpin[5] = { 13, 12, 11, 8, 10 };
uint8_t sensorvalue[5] = { 0, 0, 0, 0, 0 };
int tuning[5] = { 0, -2, 1, 2, 0 };

//proportional
int basespeed = 100;
float p = 0.0;
float kp = 25.0;
int correction;
int error = 0;
int leftspeed = 0;
int rightspeed = 0;

//cubemechanism
Servo myservo;
int cube = 3;
int pointdetect = 0;

//timer
unsigned long currenttime;
unsigned long lasttime;

enum State { NORMAL,
             HARD_LEFT,
             HARD_RIGHT,
             CHECKPOINT,
             STABILIZE
};

State state = NORMAL;

void readsensor() {
  error = 0;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(sensorpin[i]) == LOW) {
      sensorvalue[i] = 1;
      error += sensorvalue[i] * tuning[i];
    } else {
      sensorvalue[i] = 0;
    }
  }
}

void checkstate() {
  if (sensorvalue[0] == 1) {
    state = HARD_LEFT;
  }
  if (sensorvalue[4] == 1) {
    state = HARD_RIGHT;
  }
  if ((sensorvalue[0] == 1 && sensorvalue[1] == 1) && (sensorvalue[4] == 1 && sensorvalue[3] == 1)) {
    state = CHECKPOINT;
  }
}

void goforward() {
  if (currenttime - lasttime < 500) {
    digitalWrite(leftmotor1, HIGH);
    digitalWrite(leftmotor2, LOW);
    analogWrite(leftena, basespeed);

    digitalWrite(rightmotor1, HIGH);
    digitalWrite(rightmotor2, LOW);
    analogWrite(rightena, basespeed);
    return;
  }
}

void cubemechanism() {
  if (currenttime - lasttime <= 700) {
    myservo.write(90);
    delay(30);
    myservo.write(0);
  }
}

void pullmotor() {

  if (leftspeed >= 0) {
    digitalWrite(leftmotor1, HIGH);
    digitalWrite(leftmotor2, LOW);
    analogWrite(leftena, leftspeed);
  } else {
    digitalWrite(leftmotor1, LOW);
    digitalWrite(leftmotor2, HIGH);
    analogWrite(leftena, -leftspeed);
  }

  if (rightspeed >= 0) {
    digitalWrite(rightmotor1, HIGH);
    digitalWrite(rightmotor2, LOW);
    analogWrite(rightena, rightspeed);
  } else {
    digitalWrite(rightmotor1, LOW);
    digitalWrite(rightmotor2, HIGH);
    analogWrite(rightena, -rightspeed);
  }
}

void pid() {
  p = kp * error;
  correction = p;
  leftspeed = constrain(basespeed + correction, -255, 255);
  rightspeed = constrain(basespeed - correction, -255, 255);
}

void debug() {

  for (int i = 0; i < 5; i++) {
    Serial.print(sensorvalue[i]);
    Serial.print(" ");
  }

  Serial.println();

  Serial.print(" error: ");
  Serial.print(error);

  Serial.println();

  Serial.println(state);
}

void setup() {

  pinMode(leftmotor1, OUTPUT);
  pinMode(leftmotor2, OUTPUT);
  pinMode(leftena, OUTPUT);

  pinMode(rightmotor1, OUTPUT);
  pinMode(rightmotor2, OUTPUT);
  pinMode(rightena, OUTPUT);

  for (int i = 0; i < 5; i++) {
    pinMode(sensorpin[i], INPUT);
  }

  currenttime = millis();
  Serial.begin(9600);
}

void loop() {
  currenttime = millis();

  switch (state) {
    case NORMAL:
      readsensor();
      checkstate();
      pid();
      pullmotor();
      lasttime=currenttime;
      break;

    case HARD_LEFT:

      goforward();
      leftspeed = -100;
      rightspeed = 100;
      state = STABILIZE;
      break;

    case HARD_RIGHT:

      goforward();
      leftspeed = 100;
      rightspeed = -100;
      state = STABILIZE;
      break;

    case CHECKPOINT:
      if (pointdetect == 0 || pointdetect == 3) {
        goforward();
      } else {
        cubemechanism();
        pointdetect++;
      }
      break;

    case STABILIZE:
      if (sensorvalue[2] != 1) {
        pullmotor();
      } else {
        state = NORMAL;
      }
      break;
  }
}