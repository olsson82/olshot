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
 *   define for battery. On prodction comment out DEBUG
 *   to disable debug.
 *
 *---------------------------------------------**/
/**----------------------
 *    Include librarys
 *------------------------**/
#include <SPI.h>
#include "RF24.h"
/**----------------------
 *    Disable Stuff
 *------------------------**/
#define DEBUG
#define BATTERY
/**----------------------
 *    Some Variables
 *------------------------**/
int targID = 3401;  //This is the target ID
int sendID = 2401;  //This is the response ID
int sensor;
int shootMode = 0;
int batlow = 0;
long startTime, stopTime;

#ifdef BATTERY
//Change the limit in milivolt for battery before the targets will warn for low battery.
float batLevel = 3200;
unsigned long previousMillis = 0;
const long interval = 1000;
int ledState = HIGH;
#endif

struct recived {
  int I = 1;  // The id number for the target
  int J = 0;  // If it active target or test signal.
  long F = 15000; //Failsafe time
};
typedef struct recived Package;
Package intotarget;

struct sending {
  int K = 1;    // The send id number for the target
  int L = 0;    // Is recived
  float O = 1;  //Battery volt
};
typedef struct sending Package1;
Package1 outfromtarget;

RF24 myRadio(9, 10);  //Connection for nRF24L01
const uint64_t addresses[4] = { 0xF0F0F0F0E1LL, 0xABCDABCD71LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0C1LL };

const int knockSensor = A0;

int LEDPWR = 6;     //Red
int LEDTarget = 5;  //Blue

#ifdef BATTERY
long readVcc() {
/* This is used to check the battery in the targets. 
    It uses the internal system to check the power.
    If the power is to low, the red light will go on.
    Undefine BATTERY if not needed.
  */
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2);             // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);  // Start conversion
  while (bit_is_set(ADCSRA, ADSC))
    ;  // measuring

  uint8_t low = ADCL;   // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH;  // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result;  // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;               // Vcc in millivolts
}
#endif

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Setting up target");
#endif
  pinMode(LEDTarget, OUTPUT);
  digitalWrite(LEDTarget, LOW);
  pinMode(LEDPWR, OUTPUT);
  digitalWrite(LEDPWR, HIGH);

  delay(100);
  myRadio.begin();
  myRadio.setPALevel(RF24_PA_MIN);    //Use MIN on testing, change to MAX when they working good.
  myRadio.setDataRate(RF24_250KBPS);  //Lower datarate gives longer distans
  myRadio.setRetries(15, 15);
  myRadio.setChannel(108);  //Keep abow wifi channels
  myRadio.openReadingPipe(1, addresses[1]);
  myRadio.openWritingPipe(addresses[0]);
  myRadio.startListening();
  delay(100);
}

void loop() {

#ifdef BATTERY
  printVolts();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LEDPWR, ledState);
  }
#endif

  if (myRadio.available(addresses[1])) {
#ifdef DEBUG
    Serial.println("Radio Avalible");
#endif

    myRadio.read(&intotarget, sizeof(intotarget));
    if (intotarget.I == targID) {
#ifdef DEBUG
      Serial.println("Target ID " + String(intotarget.I));
#endif
      if (intotarget.J == 1) {
        //Enable the target
        shootMode = 1;
        targetHit();
      } else if (intotarget.J == 2) {
        //Enable the test
        testTransmiter();
      }
    }
  }
}

void targetHit() {
#ifdef DEBUG
  Serial.println("Start hit target");
#endif
  startTime = millis();  //Add start time for failsafe
  digitalWrite(LEDTarget, HIGH);
  while (shootMode == 1) {
    stopTime = millis();  //Count stoptime for failsafe
    if (stopTime - startTime >= intotarget.F) {
#ifdef DEBUG
      Serial.println("Activate Failsafe");
#endif
      outfromtarget.K = sendID;
      outfromtarget.L = 2;  //For report failsafe
#ifdef DEBUG
      Serial.println("Sending failsafe response");
#endif
      shootMode = 0;
      digitalWrite(LEDTarget, LOW);
    }
    sensor = analogRead(knockSensor);
    if (sensor < 1022) {
#ifdef DEBUG
      Serial.println("Knock");
#endif
      digitalWrite(LEDTarget, LOW);  //Knock is made stop light
      outfromtarget.K = sendID;
      outfromtarget.L = 1;
#ifdef DEBUG
      Serial.println("Sending recive response");
#endif
      myRadio.stopListening();
      delay(100);
      myRadio.write(&outfromtarget, sizeof(outfromtarget));
      shootMode = 0;
#ifdef DEBUG
      Serial.println("Send OK");
#endif

      delay(100);
      myRadio.startListening();
    }
  }
}

void testTransmiter() {
#ifdef DEBUG
  Serial.println("Run test signal");
#endif
  outfromtarget.K = sendID;
  outfromtarget.L = 1;
#ifdef DEBUG
  Serial.println("Sending recive response " + String(outfromtarget.K));
#endif
  myRadio.stopListening();
  delay(100);
  myRadio.write(&outfromtarget, sizeof(outfromtarget));
  delay(100);
  digitalWrite(LEDPWR, HIGH);
  digitalWrite(LEDTarget, LOW);
  delay(500);
  digitalWrite(LEDPWR, LOW);
  digitalWrite(LEDTarget, HIGH);
  delay(500);
  digitalWrite(LEDPWR, HIGH);
  digitalWrite(LEDTarget, LOW);
  delay(500);
  outfromtarget.L = 0;
  delay(100);
  myRadio.startListening();

#ifdef DEBUG
  Serial.println("Test is done");
#endif
}

#ifdef BATTERY
void printVolts() {
  /*
         * Checks the battery level and turns on red if low power.
         */

  outfromtarget.O = readVcc();
  if (outfromtarget.O < batLevel)  //In millivolt when we are going to warn on low power
  {
    batlow = 1;
  } else {
    batlow = 0;
    digitalWrite(LEDPWR, HIGH);
  }
}
#endif
