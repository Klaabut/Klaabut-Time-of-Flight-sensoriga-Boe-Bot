//Libraries
#include <Wire.h>
#include <Servo.h>
#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Initialize ToF
Adafruit_VL53L0X tof = Adafruit_VL53L0X();
#define tofPin 3

//Initialize Servos
Servo servoTof;
Servo leftWheel;
Servo rightWheel;
#define rightServoPin 5
#define leftServoPin  6
#define right 11
#define forward 12
#define left 13
#define behind 14

//Initialize buzzer
#define buzzer 2

//Initialize NRF
RF24 radio(9,10); // (CE, CSN)
const byte address[] = "00001";
int nrfData[16];

//Mapping variables
int xRobot = 500, yRobot = 500;

//Direction variables(right, forward, left, behind)
bool directionArray[4] = {false, false, false, false};
bool previousDirectionArray[4] = {false, false, false, false};
bool reachedEnd = false;

//Time variables
unsigned long time1 = 0;
unsigned long time2 = 0;

//Define constants
#define maxDistanceWall 200    //Maximum distance from the wall, so the robot won't collide with wall
#define reachedEndTime 10000   //If the robot goes straight for this amount of time, then we are outside the maze

//Sub-functions

//Sets the wheels in the starting position
void setWheels(int delayLeft, int delayRight) {
  leftWheel.writeMicroseconds(delayLeft);
  rightWheel.writeMicroseconds(delayRight);
  delay(20);
}

//Turns the robot to the left
void turnLeft(){
  setWheels(1440, 1440);
  delay(930);
  setWheels(1500, 1500);
}

//Turns the robot to the right
void turnRight(){
  setWheels(1560, 1560);
  delay(940);
  setWheels(1500, 1500);
}

//Robot will move forward
void goForward(){
  setWheels(1550, 1445);
  delay(250);
  setWheels(1500, 1500);
}

//Timer function
void timer(){
  time2 = millis();
  time2 -= time1;
}

void setup() {
  Serial.begin(9600);
  
  //NRF setup
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  //Servo setup
  servoTof.attach(tofPin);
  leftWheel.attach(leftServoPin);
  rightWheel.attach(rightServoPin);

  //Wheels setup, required so the wheels wouldn't move at startup
  setWheels(1500, 1500);

  //ToF setup
  tof.begin();

  //Buzzer setup
  pinMode(buzzer, OUTPUT);
  
}

void loop() {

  //Checks if the robot has reached the end
  if(reachedEnd == false){
    //ToF measurement data
    VL53L0X_RangingMeasurementData_t measure;
    int distance = 0, degree = 0, furthest = 0, j, i, k = 0;
  
    //Set the ToF servo to the position we want the ToF to look
    //(0 = right, 80 = straight, 160 = left)
    //servoTof.write(80);
  
    //Read the measurement data from the ToF in mm
    //tof.rangingTest(&measure, false);
    //distance = measure.RangeMilliMeter;
  
    //Turning the robot
    //turnLeft();
    //turnRight();
    //goForward();
  
    //Sends data to the other NRF (maximum is 32 bytes in one single send)
    //radio.write(&nrfData, sizeof(nrfData));
  
    //Measures right, forward, left and behind the robot and saves the data
    for(i = 0; i < 6; i += 2, degree += 80){
      //Turns the ToF servo
      servoTof.write(degree);
      delay(1000);
      
      //Checks if "k" is even or odd
      //Even, means that ToF is looking right/left
      if(k % 2 == 0){
        //Means the robot is looking right
        if(k == 0){
          tof.rangingTest(&measure, false);
          //Since it is the first time measuring then it has to be the furthest
          if(previousDirectionArray[0] == false){
            furthest = measure.RangeMilliMeter;
            //Setting the first bit to 1 in the directionArray
            for(j = 0; j < 4; j++){
              if(j == 0){
                directionArray[j] = true;
              }
              else{
                directionArray[j] = false;
              }
            }
            nrfData[i] = (measure.RangeMilliMeter / 5) + xRobot;
            nrfData[i + 1] = yRobot;
          }
        }
        //Means the robot is looking left
        else{
          tof.rangingTest(&measure, false);
          //Checking if the new measurment is further than the last
          if(measure.RangeMilliMeter > furthest && previousDirectionArray[2] == false){
            furthest = measure.RangeMilliMeter;
            for(j = 0; j < 4; j++){
              if(j == 2){
                directionArray[j] = true;
              }
              else{
                directionArray[j] = false;
              }
            }
          }
          if(previousDirectionArray[2] == false){
            nrfData[i] = xRobot - (measure.RangeMilliMeter / 5);
            nrfData[i + 1] = yRobot;
          }
        }
      }
      //Odd, means that ToF is looking forward
      else{
        tof.rangingTest(&measure, false);
        //Checking if the new measurment is further than the last
        if(measure.RangeMilliMeter > furthest && previousDirectionArray[1] == false){
          furthest = measure.RangeMilliMeter;
          for(j = 0; j < 4; j++){
            if(j == 1){
              directionArray[j] = true;
            }
            else{
              directionArray[j] = false;
            }
          }
        }
        if(previousDirectionArray[1] == false){
          nrfData[i] = xRobot;
          nrfData[i + 1] = yRobot - (measure.RangeMilliMeter / 5);
        }
      }
      k++;
    }
    turnLeft();
    //Robot is looking behind
    tof.rangingTest(&measure, false);
    //Checking if the new measurment is further than the last
    if(measure.RangeMilliMeter > furthest && previousDirectionArray[3] == false){
      furthest = measure.RangeMilliMeter;
      for(j = 0; j < 4; j++){
        if(j == 3){
          directionArray[j] = true;
        }
        else{
          directionArray[j] = false;
        }
      }
    }
    if(previousDirectionArray[3] == false){
      nrfData[6] = xRobot;
      nrfData[7] = (measure.RangeMilliMeter / 5) + yRobot;
    }
  
    //The robot's location
    nrfData[8] = xRobot;
    nrfData[9] = yRobot;
    //Send the data to the receiving NRF
    radio.write(&nrfData, sizeof(nrfData));
  
    //Move the robot to the furthest point measured
    servoTof.write(80);
    delay(1000);
    //The measurement to the right was the furthest
    if(directionArray[0] == true){
      //Save the location from where the robot came
      //Came from right, means doesn't have to look left
      for(j = 0; j < 4; j++){
        if(j == 2){
          previousDirectionArray[j] = true;
        }
        else{
          previousDirectionArray[j] = false;
        }
      }
      turnLeft();
      turnLeft();
      //Goes forward until very close to the wall
      time1 = millis();
      do{
        timer();
        if(time2 > reachedEndTime){
          reachedEnd = true;
          break;
        }
        tof.rangingTest(&measure, false);
        goForward();
      }while(measure.RangeMilliMeter >= maxDistanceWall);
      xRobot = (furthest / 5) + xRobot;
      turnLeft();
    }
    //Forward was the furthest
    else if(directionArray[1] == true){
      //Save the location from where the robot came
      //Came from forward, means doesn't have to look behind
      for(j = 0; j < 4; j++){
        if(j == 3){
          previousDirectionArray[j] = true;
        }
        else{
          previousDirectionArray[j] = false;
        }
      }
      turnRight();
      //Goes forward until very close to the wall
      time1 = millis();
      do{
        timer();
        if(time2 > reachedEndTime){
          reachedEnd = true;
          break;
        }
        tof.rangingTest(&measure, false);
        goForward();
      }while(measure.RangeMilliMeter >= maxDistanceWall);
      yRobot = yRobot - (furthest / 5);
    }
    //Left was the furthest
    else if(directionArray[2] == true){
      //Save the location from where the robot came
      //Came from left, means doesn't have to look right
      for(j = 0; j < 4; j++){
        if(j == 0){
          previousDirectionArray[j] = true;
        }
        else{
          previousDirectionArray[j] = false;
        }
      }
      //Goes forward until very close to the wall
      time1 = millis();
      do{
        timer();
        if(time2 > reachedEndTime){
          reachedEnd = true;
          break;
        }
        tof.rangingTest(&measure, false);
        goForward();
      }while(measure.RangeMilliMeter >= maxDistanceWall);
      xRobot = xRobot - (furthest / 5);
      turnRight();
    }
    //Behind was the furthest
    else if(directionArray[3] == true){
      //Save the location from where the robot came
      //Came from behind, means doesn't have to look forward
      for(j = 0; j < 4; j++){
        if(j == 1){
          previousDirectionArray[j] = true;
        }
        else{
          previousDirectionArray[j] = false;
        }
      }
      turnLeft();
      //Goes forward until very close to the wall
      time1 = millis();
      do{
        timer();
        if(time2 > reachedEndTime){
          reachedEnd = true;
          break;
        }
        tof.rangingTest(&measure, false);
        goForward();
      }while(measure.RangeMilliMeter >= maxDistanceWall);
      yRobot = (furthest / 5) + yRobot;
      turnRight();
      turnRight();
    }
  }
  //Means the robot has reached the end
  else{
    //tone(pin, sound signal in Hz)
    //noTone(pin);
    tone(buzzer, 400);
    delay(200);
    noTone(buzzer);
    delay(200);
    tone(buzzer, 600);
    delay(400);
    noTone(buzzer);
    delay(400);
  }
}
