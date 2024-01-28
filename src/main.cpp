/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LCD.h"
#include "mbed.h"
#include <string>

// Blinking rate in milliseconds
#define BLINKING_RATE 500ms

DigitalOut Enr(PC_11);
DigitalOut Enl(PC_12);
PortOut segm7(PortC, 0xF);

InterruptIn changeDirButton(PA_6); // change Dir
DigitalIn reset(PA_1);

lcd mylcd;

char tabelle[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
int enemies[2][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

int counter = 0;
int nextSpawn = 0;
bool playerUP;
int lastSpawn = 10;

bool lastRow = 0;
int spawnsInRow = 0;

void ausgabe() {
    bool dead = false;
    std::string s1;
    std::string s2;

    for (int i = 0; i < 16; i++) {
      if (enemies[0][i] == 1) {
        s1.append("<");

        if (!playerUP && i == 1) {
          dead = true;
        }
      } else {
        if (!playerUP && i == 0) {
          s1.append("|");
        } else {
          s1.append(" ");
        }
      }
    }

    for (int i = 0; i < 16; i++) {
      if (enemies[1][i] == 1) {
        s2.append("<");

        if (playerUP && i == 1) {
          dead = true;
        }
      } else {
        if (playerUP && i == 0) {
          s2.append("|");
        } else {
          s2.append(" ");
        }
      }
    }

    if (dead) {
      mylcd.cursorpos(0);
      mylcd.printf("Du bist tot :(");
      mylcd.locate(0, 1);
      mylcd.printf("Respawn: PA1");

      while (!reset);
    } else {
      mylcd.cursorpos(0);
      mylcd.printf(s1.c_str());
      mylcd.locate(0, 1);
      mylcd.printf(s2.c_str());
    }
}

void changeDir() { 
    playerUP = !playerUP;
    // ausgabe();
}

void increase() { counter++; }

void print() {
  Enl = 1;
  segm7 = tabelle[counter / 10];
  for (int i = 0; i <= 5000; i++)
    ;
  Enl = 0;

  Enr = 1;
  segm7 = tabelle[counter % 10];
  for (int i = 0; i <= 5000; i++)
    ;
  Enr = 0;
}

void moveEnemy(int enemies[]) {
  for (int i = 0; i < 16; i++) {
    int target = enemies[i];

    int newIndex = i - 1;
    if (i == 15) {
      enemies[i] = 0;
    }
    if (newIndex > 0) {
      enemies[newIndex] = target;
    }
  }
}

void moveEnemies() {
  moveEnemy(enemies[0]);
  moveEnemy(enemies[1]);
}


// Generates a random number between 0 and 10
int randomInt() { return std::rand() % (10 + 1); }

int main() {
  reset.mode(PullDown);

  changeDirButton.rise(&changeDir);
  changeDirButton.enable_irq();

  int step = 0;

  while (true) {
    int random = randomInt();


    if (++lastSpawn > nextSpawn && random < 5) {
      lastSpawn = 0;

      bool row = false;
      if (std::rand() % 2) {
        row = true;
      }

      if (lastRow == row) {
          if (++spawnsInRow > 2) {
              row = !row;
              spawnsInRow = 0;
          }
      } else {
          spawnsInRow = 0;
      }

      enemies[row][15] = 1;
      nextSpawn = 2 + randomInt() / 4;


      lastRow = row;
    }

    ausgabe();

    moveEnemies();
    ThisThread::sleep_for(80ms);
  }
}
