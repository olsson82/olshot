//======================================================
//                                                      
//   #####   ##       ####  ##   ##   #####   ######  
//  ##   ##  ##      ##     ##   ##  ##   ##    ##    
//  ##   ##  ##       ###   #######  ##   ##    ##    
//  ##   ##  ##         ##  ##   ##  ##   ##    ##    
//   #####   ######  ####   ##   ##   #####     ##    
//                                                      
//======================================================
/**------------------------------------------------------------------------
 * ?                                ABOUT
 * @author         :  Andreas Olsson
 * @email          :  andreas@olsserv82.com
 * @repo           :  https://github.com/olsson82/olshot
 * @createdOn      :  2023-01-14
 * @description    :  Code for wireless shooting sensor.
 *------------------------------------------------------------------------**/
/**----------------------------------------------
 * *                   INFO
 *   Give each sensor an unique targID and sendID
 *   If you don't need battery checker, comment out
 *   define for battery.
 *
 *---------------------------------------------**/
/**----------------------
 *    Include librarys
 *------------------------**/
#include <Arduino.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
/**----------------------
 *    Define settings
 *------------------------**/
#define DEBUG
#define BATTERY
/**----------------------
 *    SETTINGS FOR FAILSAFE
 *------------------------**/
long startTime,stopTime;
//Set how long it will stay inside a while loop until failsafe will break it.
long myDesiredTime = 15000; //1000 per secound. Default is 15000 (15 Sec)
/**----------------------
 *    Sensor ID
 *------------------------**/
int targID = 3401; //This is the target ID
int sendID = 2401; //This is the response ID

#ifdef BATTERY
//Set the level in milivolt it will warn about low battery
float batLevel = 3200;
#endif
/**----------------------
 *    Send & Recive package
 *------------------------**/
struct recived
{
  int I = 1; // The id number for the target
  int J = 0; // If it active target or test signal.
};
typedef struct recived Package;
Package incoming;

struct sending
{
  int K = 1; // The send id number for the target
  int L = 0; // Is recived
  float O = 1; //Battery volt
};
typedef struct sending Package1;
Package1 outgoing;
/**----------------------
 *    RF24 Settings
 *------------------------**/
RF24 myRadio(9, 10); //Connection for nRF24L01
const uint64_t addresses[4] = {0xF0F0F0F0E1LL, 0xABCDABCD71LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0C1LL};
/**----------------------
 *    Led Pins
 *------------------------**/
int LEDblue = 3;  //Active target
int LEDred = 5;   //Power on & blink on low battery
/**----------------------
 *    Sensor Pin
 *------------------------**/
const int knockSensor = A0;
/**----------------------
 *    For Shootmode active
 *------------------------**/
int shootMode = 0;

#ifdef BATTERY
long readVcc() {
  /**----------------------------------------------
   * *                   INFO
   *   This is used to check battery in targets.
   *   It uses the internal system to check the power.
   *   If the power is to low, the led light will blink.
   *
   *---------------------------------------------**/
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
#endif

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Setting up Target");
#endif
  pinMode(LEDblue, OUTPUT);
  digitalWrite(LEDblue, LOW);
  pinMode(LEDred, OUTPUT);
  digitalWrite(LEDred, HIGH); //Power indicator on
  /**----------------------
   *    NRF Setup
   *------------------------**/
  delay(100);
  myRadio.begin();
  myRadio.setPALevel(RF24_PA_MIN); //Use MIN on testing, change to MAX when they working good.
  myRadio.setDataRate(RF24_250KBPS); //Lower datarate gives longer distans
  myRadio.setRetries(15, 15);
  myRadio.setChannel(108); //Keep abow wifi channels
  myRadio.openReadingPipe(1, addresses[1]);
  myRadio.openWritingPipe(addresses[0]);
  myRadio.startListening();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
}
