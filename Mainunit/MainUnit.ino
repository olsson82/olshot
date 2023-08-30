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
 * @createdOn      :  2023-08-30
 * @description    :  Code for main unit
 *------------------------------------------------------------------------**/
/**----------------------------------------------
 * *                   INFO
 *   Code is still in development, and is a BETA only!
 *   The clock function is still not built in!
 *
 *---------------------------------------------**/
#include "EasyNextionLibrary.h"
#include <SPI.h>
#include "SdFat.h"
#include "RF24.h"
#define CNFFILE "cnf.dat" //Set filename here
SdFat SD;
RF24 myRadio(9, 10);
EasyNex myNex(Serial);
const uint64_t addresses[2] = { 0xF0F0F0F0E1LL, 0xABCDABCD71LL };
struct Config {
  int targets = 5;
  int maxrounds = 5;
  int maxrapid = 10;
  int totalplayer = 2;
  int timed = 10000;
  long safetime = 15000;
  int targID[5] = { 1, 2, 3, 4, 5 };
  int recID[5] = { 6, 7, 8, 9, 10 };
  char p1[11];
  char p2[11];
  char p3[11];
  char p4[11];
  char p5[11];
  char hp1[11];
  char hp2[11];
  char hp3[11];
  char hd1[17];
  char hd2[17];
  char hd3[17];
  float hq[2] = { 0, 0 };
  int hr = 0;
};
Config config;
struct sending {
  int I = 1;                      // The id number for the target
  int J = 0;                      // If it active target or test signal.
  long F = config.safetime;  //Sending the failsafe time
};
typedef struct sending Package;
Package totarget;

struct recived {
  int K = 1;    // The send id number for the target
  int L = 0;    // Is recived
  float O = 1;  //Battery volt
};
typedef struct recived Package1;
Package1 fromtarget;
File myFile;
const int buzzer = A0;
int LEDPWR = 6;

void setup() {
  pinMode(LEDPWR, OUTPUT);
  digitalWrite(LEDPWR, HIGH);
  if (!SD.begin(7)) {
    digitalWrite(LEDPWR, LOW);
    delay(1000);
    digitalWrite(LEDPWR, HIGH);
    delay(1000);
    digitalWrite(LEDPWR, LOW);
    while (1)
      ;
  }
  if (!myRadio.begin()) {
    digitalWrite(LEDPWR, LOW);
    delay(1000);
    digitalWrite(LEDPWR, HIGH);
    delay(1000);
    digitalWrite(LEDPWR, LOW);
    while (1)
      ;
  }
  myRadio.setPALevel(RF24_PA_MIN);    //Use min when testing, then change to MAX for better distance.
  myRadio.setDataRate(RF24_250KBPS);  //Better distance if sending low data.
  myRadio.setRetries(15, 15);
  myRadio.setChannel(108);  //Keep abow wifi channels
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.openWritingPipe(addresses[1]);
  myRadio.startListening();
  delay(100);
  myNex.begin(9600); 
  myNex.writeStr("page page1");
  delay(50);
  if (SD.exists(CNFFILE)) {
    conf(0);
  } else {
    conf(1);
  }
}

void loop() {
  myNex.NextionListen();
}

void trigger7() {
  int sys;
  sys = myNex.readNumber("sys0");
  if (sys == 1) {
    config.safetime = myNex.readNumber("n4.val");
    config.maxrapid = myNex.readNumber("n0.val");
    config.timed = myNex.readNumber("n2.val");
    config.maxrounds = myNex.readNumber("n1.val");
    config.targets = myNex.readNumber("n3.val");
  }
  if (sys == 2) {
    for (int ao = 0; ao < 5; ao++) {
      config.targID[ao] = myNex.readNumber("n" + String(ao) + ".val");
      config.recID[ao] = myNex.readNumber("na" + String(ao) + ".val");
    }
  }
  if (sys == 3) {
    config.totalplayer = myNex.readNumber("n0.val");
    myNex.readStr("ta0.txt").toCharArray(config.p1, 11);
    myNex.readStr("ta1.txt").toCharArray(config.p2, 11);
    myNex.readStr("ta2.txt").toCharArray(config.p3, 11);
    myNex.readStr("ta3.txt").toCharArray(config.p4, 11);
    myNex.readStr("ta4.txt").toCharArray(config.p5, 11);
  }
  myNex.writeStr("page page2");
  delay(50);
  conf(1);
}

void trigger2() {
  sendTest(1);
}

void trigger1() {
  play(myNex.readNumber("sys1"));
}

void trigger4() {
  myNex.writeNum("n4.val", config.safetime);
  myNex.writeNum("n0.val", config.maxrapid);
  myNex.writeNum("n2.val", config.timed);
  myNex.writeNum("n1.val", config.maxrounds);
  myNex.writeNum("n3.val", config.targets);
}

void trigger5() {
  for (int ao = 0; ao < 5; ao++) {
    myNex.writeNum("n" + String(ao) + ".val", config.targID[ao]);
    myNex.writeNum("na" + String(ao) + ".val", config.recID[ao]);
  }
}

void trigger6() {
  myNex.writeNum("n0.val", config.totalplayer);
  myNex.writeStr("ta0.txt", config.p1);
  myNex.writeStr("ta1.txt", config.p2);
  myNex.writeStr("ta2.txt", config.p3);
  myNex.writeStr("ta3.txt", config.p4);
  myNex.writeStr("ta4.txt", config.p5);
}

void trigger3() {
  myNex.writeStr("ta0.txt", String(config.hq[0]));
  myNex.writeStr("ta1.txt", config.hp1);
  myNex.writeStr("ta2.txt", config.hd1);
  myNex.writeStr("ta3.txt", String(config.hq[1]));
  myNex.writeStr("ta4.txt", config.hp3);
  myNex.writeStr("ta5.txt", config.hd3);
  myNex.writeStr("ta6.txt", String(config.hr));
  myNex.writeStr("ta7.txt", config.hp2);
  myNex.writeStr("ta8.txt", config.hd2);
}

void conf(int set) {
  if (set == 1) {
    SD.remove(CNFFILE);
    myFile = SD.open(CNFFILE, FILE_WRITE);
    if (myFile) {
      const size_t bytesWritten = myFile.write((byte *)&config, sizeof(config));
      myFile.close();
    }
  } else {
    myFile = SD.open(CNFFILE, FILE_READ);
    const size_t bytesRead = myFile.read((byte *)&config, sizeof(config));
    myFile.close();
  }
}

void inform(int buzz, int light, int runs) {
  for (int runn = 0; runn < runs; runn++) {
    if (buzz == 1) {
      tone(buzzer, 1000);
    }
    if (light == 1) {
      digitalWrite(LEDPWR, LOW);
    }
    delay(1000);
    if (buzz == 1) {
      noTone(buzzer);
    }
    if (light == 1) {
      digitalWrite(LEDPWR, HIGH);
    }
    delay(1000);
  }
} 

void sendTest(int RunTest) {
  long startTime, stopTime;
  while (RunTest == 1) {
    int i;
    int oTargets = config.targets - 1;
    int WaitForResponse = 0;

    for (i = 0; i <= oTargets; i++) {
      totarget.I = config.targID[i];
      totarget.J = 2;
      myRadio.stopListening();
      delay(100);
      myRadio.write(&totarget, sizeof(totarget));
      delay(100);
      myRadio.startListening();
      WaitForResponse = 1;
      startTime = millis();
      while (WaitForResponse == 1) {
        stopTime = millis();
        if (stopTime - startTime >= config.safetime) {
          WaitForResponse = 0;
          inform(1, 1, 3);
        }
        if (myRadio.available(addresses[0])) {
          myRadio.read(&fromtarget, sizeof(fromtarget));
          if (fromtarget.K == config.recID[i]) {
            myNex.writeStr("page18.bbt" + String(i) + ".txt", String(fromtarget.O));
            WaitForResponse = 0;
            delay(1000);
          }
        }
      }
      if (i >= oTargets) {
        RunTest = 0;
        delay(2000);
        myNex.writeStr("page page1");
      }
    }
  }
}

void play(int g) {
  String pname;
  for (int ao = 0; ao <= config.totalplayer - 1; ao++) {
    myNex.writeStr("page page8");
    delay(50);
    if (ao == 0) {
      myNex.writeStr("tnextp.txt", config.p1);
      pname = config.p1;
    } else if (ao == 1) {
      myNex.writeStr("tnextp.txt", config.p2);
      pname = config.p2;
    } else if (ao == 2) {
      myNex.writeStr("tnextp.txt", config.p3);
      pname = config.p3;
    } else if (ao == 3) {
      myNex.writeStr("tnextp.txt", config.p4);
      pname = config.p4;
    } else if (ao == 4) {
      myNex.writeStr("tnextp.txt", config.p5);
      pname = config.p5;
    }
    inform(1, 1, 10);
    int oTargets = config.targets;
    int currentPort = random(oTargets);
    int newPort = random(oTargets);
    int sendAway = 0;
    int missTarget = 0;
    int hitCounter = 0;
    unsigned long time1;
    unsigned long time2;
    float interval1;
    long startTime, stopTime;
    int oRounds;
    float timedRun;
    if (g == 1) {
      myNex.writeStr("page page3");
      oRounds = config.maxrounds;
    } else if (g == 2) {
      myNex.writeStr("page page9");
      interval1 = 0;
      timedRun = (float)config.timed;
    } else {
      myNex.writeStr("page page10");
      oRounds = config.maxrapid;
    }
    delay(500);
    randomSeed(millis());
    if (g == 3 || g == 2) {
      time1 = millis();
    }
    if (g == 1 || g == 3) {
      for (int i = 0; i < oRounds; i++) {
        if (g == 1) {
          myNex.writeStr("tround.txt", String(i + 1));
          delay(random(3000) + 1000);
          time1 = millis();
        }
        totarget.I = config.targID[currentPort];
        totarget.J = 1;
        fromtarget.K = config.recID[currentPort];
        sendAway = 1;
        myRadio.stopListening();
        delay(100);
        myRadio.write(&totarget, sizeof(totarget));
        delay(100);
        myRadio.startListening();
        startTime = millis();
        while (sendAway == 1) {
          stopTime = millis();
          if (stopTime - startTime >= config.safetime) {
            sendAway = 0;
            missTarget = 1;
          }
          if (myRadio.available(addresses[0])) {
            myRadio.read(&fromtarget, sizeof(fromtarget));
            if (fromtarget.K == config.recID[currentPort]) {
              myNex.writeStr("page18.bbt" + String(currentPort) + ".txt", String(fromtarget.O));
              if (fromtarget.L == 2) {
                missTarget = 1;
              }
              sendAway = 0;
            }
          }
        }
        if (missTarget == 1) {
          missTarget = 0;
          i = i - 1;
          if (g == 3) {
            newPort = random(oTargets);
            while (newPort == currentPort)
              newPort = random(oTargets);
            currentPort = newPort;
          }
        } else {
          if (g == 1) {
            time2 = millis();
            interval1 = (time2 - time1);
            interval1 = interval1 / 1000;
            myNex.writeStr("trtime.txt", String(interval1));
            myNex.writeStr("page12.rrq" + String(i + 1) + ".txt", String(interval1));
            savehigh(1, interval1, pname, 0);
          } else {
            newPort = random(oTargets);
            while (newPort == currentPort)
              newPort = random(oTargets);
            currentPort = newPort;
          }
        }
        if (g == 1) {
          newPort = random(oTargets);
          while (newPort == currentPort)
            newPort = random(oTargets);
          currentPort = newPort;
        }

      }
      if (g == 3) {
        time2 = millis();
        interval1 = (time2 - time1);
        interval1 = interval1 / 1000;
        myNex.writeStr("page14.rtrap1.txt", String(interval1));
        savehigh(3, interval1, pname, 0);
        myNex.writeStr("page page14");
        delay(20000);
      } else {
        myNex.writeStr("page page12");
        delay(20000);
      }

    } else {
      while (interval1 < timedRun) {
        totarget.I = config.targID[currentPort];
        totarget.J = 1;
        fromtarget.K = config.recID[currentPort];
        sendAway = 1;
        myRadio.stopListening();
        delay(100);
        myRadio.write(&totarget, sizeof(totarget));
        delay(100);
        myRadio.startListening();
        startTime = millis();
        while (sendAway == 1) {
          stopTime = millis();
          if (stopTime - startTime >= config.safetime) {
            sendAway = 0;
            missTarget = 1;
          }
          if (myRadio.available(addresses[0])) {
            myRadio.read(&fromtarget, sizeof(fromtarget));
            if (fromtarget.K == config.recID[currentPort]) {
              myNex.writeStr("page18.bbt" + String(currentPort) + ".txt", String(fromtarget.O));
              if (fromtarget.L == 2) {
                missTarget = 1;
              }
              sendAway = 0;
            }
          }
        }
        if (missTarget == 1) {
          missTarget = 0;
          time2 = millis();
          interval1 = (time2 - time1);
        } else {
          hitCounter++;
          time2 = millis();
          interval1 = (time2 - time1);
          myNex.writeStr("thits.txt", String(hitCounter));
          myNex.writeStr("page13.rtih1.txt", String(hitCounter));
          savehigh(2, interval1, pname, hitCounter);
        }
        newPort = random(oTargets);
        while (newPort == currentPort)
          newPort = random(oTargets);
        currentPort = newPort;
      }
      myNex.writeStr("page page13");
      delay(20000);
    }
  }
  myNex.writeStr("page page1");
}

void savehigh(int game, float time, String play, int hits) {
  if (game == 1) {
    //Quicktime
    if (config.hq[0] == 0) {
      config.hq[0] = time;
      play.toCharArray(config.hp1, 11);
    } else if (time < config.hq[0]) {
      config.hq[0] = time;
      play.toCharArray(config.hp1, 11);
    }
  } else if (game == 2) {
    //Timed
    if (hits > config.hr) {
      config.hr = hits;
      play.toCharArray(config.hp2, 11);
    }
  } else {
    //Rapid
    if (config.hq[1] == 0) {
      config.hq[1] = time;
      play.toCharArray(config.hp3, 11);
    } else if (time < config.hq[1]) {
      config.hq[1] = time;
      play.toCharArray(config.hp3, 11);
    }
  }
  conf(1);
}