#include <Wire.h>
#include <Servo.h>
#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define right_servo_pin 5
#define left_servo_pin  6
#define tof 3

#define right 11
#define forward 12
#define left 13
#define behind 14

Servo myservo;
Servo g_left_wheel;
Servo g_right_wheel;
float pos = 0;
int turn = 1000;
float distance = 0;
int range;

int Array[10];
int RoboLocation[10];
int xRobot = 500, yRobot = 500, xWallForward, yWallForward, xWallLeft, yWallLeft; //Coordinates for the robot and the wall

//NRF stuff
RF24 radio(9,10); // CE, CSN
int data[4];
const byte address[] = "00001";

//====================Sub-Functions====================//

void setWheels(int delay_left = 1500, int delay_right = 1500) {
  g_left_wheel.writeMicroseconds(delay_left);
  g_right_wheel.writeMicroseconds(delay_right);
  delay(20);
}

void turnleft(){
  setWheels(1440, 1440);
  delay(turn);
  setWheels(1500, 1500);
}

void turnright(){
  setWheels(1560, 1560);
  delay(turn);
  setWheels(1500, 1500);
}

void turnaround(){
  setWheels(1560, 1560);
  delay(1800);
  setWheels(1500, 1500);
}

void goforward(){
  VL53L0X_RangingMeasurementData_t measure;
  distance = measure.RangeMilliMeter;
  while(distance >= 150){
    lox.rangingTest(&measure, false);
    setWheels(1560, 1440);
    distance = measure.RangeMilliMeter;
    Serial.println(distance);
  }
  //delay(600);
  
  setWheels(1500, 1500);
}


//====================SETUP====================//
void setup() {
  Serial.begin(115200);
  randomSeed(1);
  //Radio begin stuff
  radio.begin();
  //radio.setChannel(115);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
 // radio.setDataRate( RF42_250KBS );
  radio.stopListening();
  
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
  
  myservo.attach(tof);
  g_left_wheel.attach(left_servo_pin);
  g_right_wheel.attach(right_servo_pin);
  
  setWheels();

  
}

//===================================================MAIN LOOP===========================================================//

void loop() {
  int i = 0;
  int furthest = 0;
  int ajutine = 0;
  VL53L0X_RangingMeasurementData_t measure;

  while(true){
  //follow the left wall algorythm
    myservo.write(180);
    delay(1000);
        Serial.print("Reading left wall measurement... ");
        lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
        if (measure.RangeStatus != 4) {  // phase failures have incorrect data
            Serial.print("Distance (mm): ");
            range = measure.RangeMilliMeter;
            Serial.println(measure.RangeMilliMeter);
            if(range == 8191)
                Array[0] = 0;
            else
                Array[0] = range;
            Serial.println(Array[0]);
        } else {
              Serial.println(" out of range / 0");
              Array[0] = 0;
        }
    myservo.write(90);
    delay(750);
        Serial.print("Reading forward measurement... ");
        lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
        if (measure.RangeStatus != 4) {  // phase failures have incorrect data
            Serial.print("Distance (mm): ");
            range = measure.RangeMilliMeter;
            Serial.println(measure.RangeMilliMeter);
            if(range == 8191)
                Array[1] = 0;
            else
                Array[1] = range;
            Serial.println(Array[1]);
        } else {
              Serial.println(" out of range / 0");
              Array[1] = 0;
        }

    
    RoboLocation[8] = xRobot;
    RoboLocation[9] = yRobot;

    Serial.println("Saadan terve Array");
    radio.write(&RoboLocation, sizeof(RoboLocation));
    

    if(Array[0] > 250){
      //setWheels(1530, 1470);  //otse
      //delay(3000);
      Serial.println("vasakule");
      setWheels(1500, 1470);  //keera vasakule 90 kraadi
      delay(3800);
      setWheels(1530, 1470);  //otse
      delay(1000);
    }

    else if(Array[1] <= 150){
      Serial.println("paremale");
      setWheels(1530, 1530);  //keera paremale 90 kraadi
      delay(1800);
      setWheels(1500, 1500);
    }
    
    else if(Array[0] >= 140 && Array[0] <= 150 && Array[1] >= 150){ //soida otse
      Serial.println("otse");
      setWheels(1530, 1470);
      yRobot += 20;
    }    
    
    else if(Array[0] < 150){
      setWheels(1530, 1500);  //keera veidi paremale
      delay(150);
      setWheels(1530, 1470);  //otse
      delay(500);
    }

    else if(Array[0] > 160){
      setWheels(1470, 1500);  //keera veidi vasakule
      delay(150);
      setWheels(1530, 1470);  //otse
      delay(500);
    }

    
  }             //================End of wall following algorythm==============

  Serial.println("jama");

  
  for (pos = 0; pos <= 180; pos += 90) {    //Esimeed 3 m66tmist, paremalt, otse, vasakult
    myservo.write(pos);
    delay(750);
    Serial.print("Reading a measurement... ");
    lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      Serial.print("Distance (mm): ");
      range = measure.RangeMilliMeter;
      Serial.println(range);
      if(range == 8191)
        Array[i] = 0;
      else
        Array[i] = (range);
    //Serial.println(Array[i]);
    } else {
      Serial.println(" out of range / 0");
      Array[i] = 0;
    }
    
    i++;
    
    delay(750);
  }

  turnleft(); //measure once behind the robot
  delay(750);
  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      Serial.print("Distance (mm): ");
      range = measure.RangeMilliMeter;
      Serial.println(range);
      if(range == 8191)
        Array[i] = 0;
      else
        Array[i] = (range);
    //Serial.println(Array[i]);
    } else {
      Serial.println(" out of range ");
      Array[i] = 0;
    }
    
  //Send data through NRF
  Serial.println("Saadan terve Array");
  //radio.write(&Array, sizeof(Array));
  Serial.println("Saadetud");
  delay(1000);
  
  /*for (pos = 180; pos >= 0; pos -= 2) {
    myservo.write(pos);
    //VL53L0X_RangingMeasurementData_t measure;
  
    Serial.print("Reading a measurement... ");
    lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  
    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    } else {
      Serial.println(" out of range ");
    }
    delay(50);
  }*/
  
 myservo.write(90);//TOF servo
 delay(500);

 if(true){
  randomSeed(analogRead(4));
  Serial.println("Randoomne suund");
    int i = random(3);
    furthest = Array[i];
 }
 else{
    furthest = Array[0];
    for(i=0; i<4; i++){
      if(Array[i] > furthest){
        furthest = Array[i];
      }
    }
    Serial.print("Kaugeim : ");
    Serial.println(furthest);
 }
  
 
    if(furthest == Array[0]){
      turnaround();
      //myservo.write(90);
      delay(50);
      goforward();
    }
    else if(furthest == Array[1]){
      turnright();
      delay(50);
      goforward();
    }
    else if(furthest == Array[2]){
      goforward();
    }
    else if(furthest == Array[3]){
      turnleft();
      delay(50);
      goforward();
    }
  
  delay(5000);
 
 Serial.println("=======================================");
}

 
