#include <Servo.h>

#define servopin 10
#define leftmotor1 4
#define leftmotor2 3
#define leftena 5
#define rightena 6
#define rightmotor1 8
#define rightmotor2 7

//sensor
const int sensorpin[5] = { A6, A5, A4, A3, A2 };
uint8_t sensorvalue[5] = { 0, 0, 0, 0, 0 };
int threshold = 200;

//initialize variables
int basespeed = 100;
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
             HARD_LEFT_START,
             HARD_LEFT,
             HARD_RIGHT_START,
             HARD_RIGHT,
             CHECKPOINT,
             STABILIZE,
             SLIGHT_LEFT,
             SLIGHT_RIGHT,
};

State state = NORMAL;


void readsensor() {
  for (int i = 0; i < 5; i++) {
    if (analogRead(sensorpin[i]) >= threshold) {
      sensorvalue[i] = 1;
    } else {
      sensorvalue[i] = 0;
    }
  }
}

void checkstate() {
  if ((sensorvalue[0] == 1 && sensorvalue[1] == 1) && (sensorvalue[4] == 1 && sensorvalue[3] == 1)) {
    state = CHECKPOINT;
    return;
  }
  if (sensorvalue[0] == 1) {
    state = HARD_LEFT_START;
    return;
  }
  if (sensorvalue[4] == 1) {
    state = HARD_RIGHT_START;
    return;
  }
  if (sensorvalue[1] == 1) {
    state = SLIGHT_LEFT;
    return;
  }
  if (sensorvalue[3] == 1) {
    state = SLIGHT_RIGHT;
    return;
  }
}

void goforward() {
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  analogWrite(leftena, basespeed);

  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
  analogWrite(rightena, basespeed);
}

void cubemechanism() {
  if (currenttime - lasttime <= 100) {
    myservo.write(90);
  } else {
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

void debug() {

  for (int i = 0; i < 5; i++) {
    Serial.print(sensorvalue[i]);
    Serial.print(" ");
  }

  Serial.print("       ");

  switch (state) {
    case 0:
      Serial.print("NORMAL");
      break;

    case 1:
      Serial.print("HARD LEFT START");
      break;

    case 2:
      Serial.print("HARD LEFT");
      break;

    case 3:
      Serial.print("HARD RIGHT START");
      break;

    case 4:
      Serial.print("HARD RIGHT");
      break;

    case 5:
      Serial.print("CHECKPOINT");
      break;

    case 6:
      Serial.print("STABILIZE");
      break;

    case 7:
      Serial.print("SLIGHT LEFT");
      break;

    case 8:
      Serial.print("SLIGHT RIGHT");
      break;
  }
  Serial.print("     leftspeed: ");
  Serial.print(leftspeed);
  Serial.print("     rightspeed: ");
  Serial.print(rightspeed);
  Serial.print("     analogsensorreading: ");
  Serial.print(analogRead(sensorpin[2]));
  Serial.print("     pointdetect:");
  Serial.print(pointdetect);
  Serial.println();
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
  readsensor();
  checkstate();
  //debug();


  switch (state) {
    case NORMAL:
      readsensor();
      checkstate();
      leftspeed = rightspeed = basespeed;
      pullmotor();
      lasttime = currenttime;
      break;

    case HARD_LEFT_START:
      if (currenttime - lasttime < 50) {
        goforward();
      } else {
        state = HARD_LEFT;
        lasttime = currenttime;
      }
      break;

    case HARD_LEFT:

      leftspeed = -basespeed;
      rightspeed = basespeed;
      state = STABILIZE;
      break;

    case HARD_RIGHT_START:
      if (currenttime - lasttime < 50) {
        goforward();
      } else {
        state = HARD_RIGHT;
        lasttime = currenttime;
      }
      break;

    case HARD_RIGHT:

      leftspeed = basespeed;
      rightspeed = -basespeed;
      state = STABILIZE;
      break;

    case CHECKPOINT:
      if (pointdetect == 0) {
        leftspeed = 100;
        rightspeed = 100;
        pullmotor();
        if (sensorvalue[2] == 1 && sensorvalue[0] != 1 && sensorvalue[1] != 1 && sensorvalue[3] != 1 && sensorvalue[3] != 1) {
          pointdetect++;
          state = NORMAL;
        }
      } else if (pointdetect >= 3) {
        leftspeed = 100;
        rightspeed = 100;
        pullmotor();
      } else {
        leftspeed = 20;
        rightspeed = 20;
        cubemechanism();
        pointdetect++;
        state = NORMAL;
      }
      break;

    case SLIGHT_LEFT:

      leftspeed = 70;
      rightspeed = basespeed;
      state = STABILIZE;
      break;

    case SLIGHT_RIGHT:

      leftspeed = basespeed;
      rightspeed = 70;
      state = STABILIZE;
      break;

    case STABILIZE:
      readsensor();
      if (sensorvalue[2] == 1) {
        state = NORMAL;
        break;
      }

      pullmotor();

      break;
  }
}
