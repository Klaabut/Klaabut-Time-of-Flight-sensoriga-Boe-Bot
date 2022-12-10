//https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
//Faili salvestamiseks kasuta RealTerm
//(Port tab) Vali port ja õige baud rate, pane tick "Receive"
//(Capture tab) Start:Overwrite(vali õige file destination)

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Initialize NRF
RF24 radio(7, 8); // (CE, CSN)
const byte address[6] = "00001";
int nrfData[16];

void setup() {
  Serial.begin(9600);

  //NRF setup
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  
}

void loop() {
  if (radio.available()) {
    radio.read(&nrfData, sizeof(nrfData));
  }
  for(int i = 0; i < 16; i++){
      Serial.print(nrfData[i]);
      Serial.print(' ');
  }
  Serial.println();
}
