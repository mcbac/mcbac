// Created by: WestleyR
// Email(s): westleyr@nym.hush.com
// Last modifyed date: Mar 2, 2020
// Version-1.0.0
//
// This file is part of the mcbac software:
// https://github.com/WestleyR/mcbac
//
// Which that software and this file is licensed under:
//
// The Clear BSD License
//
// Copyright (c) 2020 WestleyR
// All rights reserved.
//
// This software is licensed under a Clear BSD License.
//

#include <Wire.h>
#include "LiquidCrystal_I2C.h"

#define buttonEnter A0
#define buttonBack A1
#define buttonUp A2
#define buttonDown A3

#define R_CW 2
#define R_CC 3

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  pinMode(buttonEnter, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  pinMode(R_CW, INPUT_PULLUP);
  pinMode(R_CC, INPUT_PULLUP);

  // Initialize the LCD
  lcd.begin();

  attachInterrupt(0, cursorCW, RISING);
  attachInterrupt(1, cursorCC, RISING);

  // For debugging
  Serial.begin(9600);

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MCBAC V1.0.0");
  lcd.setCursor(0, 1);
  lcd.print("Mar 3, 2020");
  delay(2500);
}

// When 1; update the LCD
int updateDisplay = 1;

// For the rotory encoder
volatile int lastCW;
volatile int lastCC;
volatile int cursorCount;
volatile int cursorRow;

// To remember the battery program
// 1 = lipo
// 2 = ninh
// ...
int batteryProg;

int enterB = 0;
int backB = 0;
int upB = 0;
int downB = 0;

void resetAllButtons() {
  enterB = 0;
  backB = 0;
  upB = 0;
  downB = 0;
}

void cursorCW() {
  cli();
  // Read pins D, and only keep pins 2 and 3
  byte reading = PIND & B00001100;
  if (reading == B00001100 && lastCW == 1) {
    cursorCount++;
    updateDisplay = 1;
    lastCW = 0;
    lastCC = 0;
  } else if (reading == B00001000) {
    lastCC = 1;
  }
  sei();
}

void cursorCC() {
  cli();
  // Read pins D, and only keep pins 2 and 3
  byte reading = PIND & B00001100;
  if (reading == B00001100 && lastCC == 1) {
    cursorCount--;
    updateDisplay = 1;
    if (cursorCount <= 0) cursorCount = 0;
    lastCC = 0;
    lastCW = 0;
  } else if (reading == B00000100) {
    lastCW = 1;
  }
  sei();
}

int checkButtons() {
  if (digitalRead(buttonEnter) == LOW) {
    enterB = 1;
    delay(100);
    return 1;
  } else if (digitalRead(buttonBack) == LOW) {
    backB = 1;
    delay(100);
    return 2;
  } else if (digitalRead(buttonUp) == LOW) {
    upB = 1;
    delay(100);
    return 3;
  } else if (digitalRead(buttonDown) == LOW) {
    downB = 1;
    delay(100);
    return 4;
  }
  // Maybe dont do this:
  //resetAllButtons();
  return 0;
}

// The function that charges a battery
void chargeLi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("lipo charge");
  // Stop if the back button is pushed
  while (digitalRead(buttonBack) == HIGH) {
    checkButtons();
  }

  // Print the results for the charge
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Results:");
  lcd.setCursor(0, 1);
  lcd.print("nothing here...");

  // Wait for the user to see it
  while (digitalRead(buttonBack) == HIGH) {
    if (checkButtons() != 0) {
      // A button was pushed
      resetAllButtons();
      break;
    }
  }
}

void chargeBattery(int prog) {
  if (prog == 1) {
    chargeLi();
  } else if (prog == 2) {
    // TODO:
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("unknown battery type");
    lcd.print(prog);
  }
}

int batteryRun;
int batteryCurrent = 100;
float batteryLIPOEndVolts = 4.15;

void configCharge(int batteryType) {
  if (batteryType == 1) {
    // Lithium battery
    while (digitalRead(buttonBack) == HIGH) {
      if (updateDisplay == 1) {
        if (cursorCount == 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Lipo/li-ion");
          lcd.setCursor(0, 2);
          lcd.print("Noninal = 3.6-3.7v");
          lcd.setCursor(0, 3);
          lcd.print("Default end = 4.2v");
          updateDisplay = 0;
          batteryProg = 1;
          Serial.println("Lipo/li-ion");
        } else if (cursorCount == 1) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("LiFEPO4");
          lcd.setCursor(0, 2);
          lcd.print("Noninal = 3.2v");
          lcd.setCursor(0, 3);
          lcd.print("Default end = 3.6v");
          updateDisplay = 0;
          batteryProg = 2;
          Serial.println("LiFEPO4");
        } else if (cursorCount == 2) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("LTO");
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ?v");
          lcd.setCursor(0, 3);
          lcd.print("Default end = ?v");
          updateDisplay = 0;
          batteryProg = 3;
          Serial.println("LTO");
        } else {
          cursorCount = 0;
          updateDisplay = 1;
        }
      }
      checkButtons();

      if (enterB == 1) {
        enterB = 0;
        cursorCount = 0;
        updateDisplay = 0;
        // Ask for charge, discharge, or cycle test
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Charge");
        lcd.setCursor(0, 1);
        lcd.print("  Discharge");
        lcd.setCursor(0, 2);
        lcd.print("  Cycle test");
        while (digitalRead(backB) == HIGH) {
          if (cursorCount == 0) {
            lcd.setCursor(0, 1);
            lcd.print("  ");
            lcd.setCursor(0, 2);
            lcd.print("  ");
            lcd.setCursor(0, 0);
            lcd.print("> ");
            updateDisplay = 0;
            batteryRun = 1;
          } else if (cursorCount == 1) {
            lcd.setCursor(0, 0);
            lcd.print("  ");
            lcd.setCursor(0, 2);
            lcd.print("  ");
            lcd.setCursor(0, 1);
            lcd.print("> ");
            updateDisplay = 0;
            batteryRun = 2;
          } else if (cursorCount == 2) {
            lcd.setCursor(0, 0);
            lcd.print("  ");
            lcd.setCursor(0, 1);
            lcd.print("  ");
            lcd.setCursor(0, 2);
            lcd.print("> ");
            updateDisplay = 0;
            batteryRun = 3;
          } else {
            cursorCount = 0;
            updateDisplay = 1;
          }
          checkButtons();
          if (enterB == 1) {
            enterB = 0;
            cursorCount = 0;
            updateDisplay = 1;
            if (batteryRun = 1) {
              // Charge
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("  Charge MA: ");
              lcd.print(batteryCurrent);
              lcd.setCursor(0, 1);
              lcd.print("  End volts: ");
              lcd.print(batteryLIPOEndVolts);
              int batteryAdjust;

              while (digitalRead(backB) == HIGH) {
                if (cursorCount == 0) {
                  lcd.setCursor(0, 1);
                  lcd.print("  ");
                  lcd.setCursor(0, 2);
                  lcd.print("  ");
                  lcd.setCursor(0, 0);
                  lcd.print("> ");
                  updateDisplay = 0;
                  batteryAdjust = 1;
                } else if (cursorCount == 1) {
                  lcd.setCursor(0, 0);
                  lcd.print("  ");
                  lcd.setCursor(0, 2);
                  lcd.print("  ");
                  lcd.setCursor(0, 1);
                  lcd.print("> ");
                  updateDisplay = 0;
                  batteryAdjust = 2;
                } else {
                  cursorCount = 0;
                  updateDisplay = 1;
                }
                checkButtons();
                if (enterB == 1) {
                  enterB = 0;
                  if (batteryAdjust == 1) {
                    cursorCount = 10;
                    //lcd.cursor();
                    lcd.setCursor(16, 0);
                    lcd.blink();
                    while (digitalRead(backB) == HIGH) {
                      if (updateDisplay == 1) {
                        lcd.setCursor(12, 0);
                        lcd.print("        ");
                        lcd.setCursor(13, 0);
                        lcd.print(cursorCount*10);
                        updateDisplay = 0;
                      }
                      checkButtons();
                      if (enterB == 1) {
                        batteryCurrent = cursorCount*10;
                        enterB = 0;
                        batteryAdjust = 0;
                        break;
                      }
                    }
                  } else if (batteryAdjust == 2) {
                    cursorCount = 100;
                    while (digitalRead(backB) == HIGH) {
                      batteryCurrent = cursorCount;
                      checkButtons();
                      if (enterB == 1) {
                        enterB = 0;
                        batteryAdjust = 0;
                        break;
                      }
                    }
                  }
                } else if (enterB == 1) {
                  enterB = 0;
                  if (batteryAdjust == 1) {
                    cursorCount = 100;
                    while (digitalRead(backB) == HIGH) {
                      batteryCurrent = cursorCount;
                      checkButtons();
                      if (enterB == 1) {
                        enterB = 0;
                        break;
                      }
                    }
                  }
                }
              }
            } else if (batteryRun == 2) {
              // Discharge
            } else if (batteryRun == 3) {
              // Cycle test
            }
          }
        }
      }
    }
    cursorCount = 0;
    updateDisplay = 1;
  } else if (batteryType == 2) {
    // Nickel battery
  } else if (batteryType == 3) {
    // Lead Acid battery
  } else if (batteryType == 4) {
    // Power supply mode
  } else if (batteryType == 5) {
    // DC load mode
  } else {
    // panic
    Serial.println("ERROR: invalid mode");
  }
}

void mainMenu() {
  if (updateDisplay == 1) {
    if (cursorCount == 0) {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Lithium");
      lcd.setCursor(5, 2);
      lcd.print("batteries");
      batteryProg = 1;
      updateDisplay = 0;
      Serial.println("Lithium battery");
    } else if (cursorCount == 1) {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Nickel");
      lcd.setCursor(5, 2);
      lcd.print("batteries");
      batteryProg = 2;
      Serial.println("Nickel battery");
      updateDisplay = 0;
    } else if (cursorCount == 2) {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Lead acid");
      lcd.setCursor(5, 2);
      lcd.print("batteries");
      batteryProg = 3;
      Serial.println("Lead acid battery");
      updateDisplay = 0;
    } else if (cursorCount == 3) {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Power");
      lcd.setCursor(5, 2);
      lcd.print("Supply");
      batteryProg = 4;
      Serial.println("Power Supply");
      updateDisplay = 0;
    } else if (cursorCount == 4) {
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("DC Load");
      batteryProg = 5;
      Serial.println("DC Load");
      updateDisplay = 0;
    } else {
      cursorCount = 0;
      updateDisplay = 1;
    }
  }

  if (enterB == 1) {
    enterB = 0;
    cursorCount = 0;
    updateDisplay = 1;
    Serial.print("Configging battery type: ");
    Serial.println(batteryProg);
    configCharge(batteryProg);
  }
}

void loop() {
  checkButtons();
  mainMenu();
}

// vim: tabstop=2 shiftwidth=2 expandtab autoindent softtabstop=0
