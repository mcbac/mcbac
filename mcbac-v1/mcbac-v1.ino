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

void setLCDPointer(int line) {
  if (line == 0) {
    lcd.setCursor(0, 0);
    lcd.print("> ");
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.setCursor(0, 2);
    lcd.print("  ");
    lcd.setCursor(0, 3);
    lcd.print("  ");
  } else if (line == 1) {
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("> ");
    lcd.setCursor(0, 2);
    lcd.print("  ");
    lcd.setCursor(0, 3);
    lcd.print("  ");
  } else if (line == 2) {
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.setCursor(0, 2);
    lcd.print("> ");
    lcd.setCursor(0, 3);
    lcd.print("  ");
  } else if (line == 3) {
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.setCursor(0, 2);
    lcd.print("  ");
    lcd.setCursor(0, 3);
    lcd.print("> ");
  }
}

int batteryRun;
int batteryCurrent = 100;
int safetyTimer = 120;
int batteryCutoff = 100;
float batteryNom;
float batteryEndVolts;
String batteryName;

void chargeCVBattery(String batteryName, int batteryCurrent, float batteryEndVolts, int batteryCutoff, int safetyTimer) {
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("CHARGING");
      lcd.setCursor(2, 2);
      lcd.print("comming soon");
      updateDisplay = 0;
    }
    checkButtons();
    //if (enterB == 1) {
    //  enterB = 0;
    //  //break;
    //}
  }
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("Done charging");
  lcd.setCursor(0, 1);
  lcd.print("mah na");

  while (digitalRead(buttonBack) == HIGH) {
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      break;
    }
  }

  updateDisplay = 1;
}

void chargeLiBattery(int batteryRun, String batteryName, int batteryCurrent, float batteryEndVolts, int safetyTimer, int batteryCutoff) {
  if (batteryRun = 1) {
    // Charge
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Charge MA: ");
    lcd.print(batteryCurrent);
    lcd.setCursor(0, 1);
    lcd.print("  End volts: ");
    lcd.print(batteryEndVolts);
    lcd.setCursor(0, 2);
    lcd.print("  Cutoff MA: ");
    lcd.print(batteryCutoff);
    lcd.setCursor(0, 3);
    lcd.print("  Safety timer: ");
    lcd.print(safetyTimer);
    int batteryAdjust;

    while (digitalRead(buttonBack) == HIGH) {
      if (cursorCount == 0) {
        setLCDPointer(0);
        updateDisplay = 0;
        batteryAdjust = 1;
      } else if (cursorCount == 1) {
        setLCDPointer(1);
        updateDisplay = 0;
        batteryAdjust = 2;
      } else if (cursorCount == 2) {
        setLCDPointer(2);
        updateDisplay = 0;
        batteryAdjust = 3;
      } else if (cursorCount == 3) {
        setLCDPointer(3);
        updateDisplay = 0;
        batteryAdjust = 4;
      } else if (cursorCount == 4) {
        lcd.setCursor(0, 0);
        lcd.print("  ");
        lcd.setCursor(0, 1);
        lcd.print("G ");
        lcd.setCursor(0, 2);
        lcd.print("O ");
        lcd.setCursor(0, 3);
        lcd.print("  ");
        updateDisplay = 0;
        batteryAdjust = 5;
      } else {
        cursorCount = 0;
        updateDisplay = 1;
      }
      checkButtons();
      if (enterB == 1) {
        int lastCursorState = cursorCount;
        enterB = 0;
        if (batteryAdjust == 1) {
          cursorCount = 10;
          lcd.setCursor(16, 0);
          lcd.blink();
          // Wait for the user to set the charge current
          while (digitalRead(buttonBack) == HIGH) {
            if (updateDisplay == 1) {
              lcd.setCursor(12, 0);
              lcd.print("        ");
              lcd.setCursor(13, 0);
              lcd.print(cursorCount * 10);
              updateDisplay = 0;
            }
            checkButtons();
            if (enterB == 1) {
              batteryCurrent = cursorCount * 10;
              enterB = 0;
              batteryAdjust = 0;
              break;
            }
          }
          lcd.noBlink();
        } else if (batteryAdjust == 2) {
          // Set the end voltage
          enterB = 0;
          cursorCount = 128;
          int lastCount = cursorCount;
          lcd.setCursor(16, 1);
          lcd.blink();
          // Wait for the user to set the end voltage
          while (digitalRead(buttonBack) == HIGH) {
            if (updateDisplay == 1) {
              lcd.setCursor(12, 1);
              lcd.print("        ");
              lcd.setCursor(14, 1);
              lcd.print(batteryEndVolts);
              if (lastCount > cursorCount) {
                batteryEndVolts -= 0.01;
                lastCount = cursorCount;
              } else if (lastCount < cursorCount) {
                batteryEndVolts += 0.01;
                lastCount = cursorCount;
              }
              updateDisplay = 0;
            }
            checkButtons();
            if (enterB == 1) {
              batteryCurrent = cursorCount / 10;
              enterB = 0;
              batteryAdjust = 0;
              break;
            }
          }
          lcd.noBlink();
        } else if (batteryAdjust == 3) {
          // Set the end voltage
          enterB = 0;
          cursorCount = 100;
          int lastCount = cursorCount;
          lcd.setCursor(16, 2);
          lcd.blink();
          // Wait for the user to set the end voltage
          while (digitalRead(buttonBack) == HIGH) {
            if (updateDisplay == 1) {
              lcd.setCursor(12, 2);
              lcd.print("        ");
              lcd.setCursor(14, 2);
              lcd.print(cursorCount);
              updateDisplay = 0;
            }
            checkButtons();
            if (enterB == 1) {
              batteryCutoff = cursorCount;
              enterB = 0;
              batteryAdjust = 0;
              break;
            }
          }
          lcd.noBlink();
        } else if (batteryAdjust == 4) {
          // Set the end voltage
          enterB = 0;
          cursorCount = 120;
          int lastCount = cursorCount;
          lcd.setCursor(16, 3);
          lcd.blink();
          // Wait for the user to set the end voltage
          while (digitalRead(buttonBack) == HIGH) {
            if (updateDisplay == 1) {
              lcd.setCursor(14, 3);
              lcd.print("      ");
              lcd.setCursor(16, 3);
              lcd.print(cursorCount);
              updateDisplay = 0;
            }
            checkButtons();
            if (enterB == 1) {
              safetyTimer = cursorCount;
              enterB = 0;
              batteryAdjust = 0;
              break;
            }
          }
        } else if (batteryAdjust == 5) {
          // Set the end voltage
          updateDisplay = 1;
          enterB = 0;
          cursorCount = 0;
          chargeCVBattery(batteryName, batteryCurrent, batteryEndVolts, batteryCutoff, safetyTimer);
          return;
        }
        cursorCount = lastCursorState;
        // Maybe run: lcd.noBlink(); here
      }
    }
  } else if (batteryRun == 2) {
    // Discharge
  } else if (batteryRun == 3) {
    // Cycle test
  }

  updateDisplay = 1;
  cursorCount = 0;
}

void configCharge(int batteryType) {
  if (batteryType == 1) {
    // Lithium battery
    while (digitalRead(buttonBack) == HIGH) {
      if (updateDisplay == 1) {
        if (cursorCount == 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "Lipo/li-ion";
          batteryNom = 3.70;
          batteryEndVolts = 4.20;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = ");
          lcd.print(batteryEndVolts);
          updateDisplay = 0;
          batteryProg = 1;
          Serial.println("Lipo/li-ion");
        } else if (cursorCount == 1) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "LiFePO4";
          batteryNom = 3.3;
          batteryEndVolts = 3.65;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = ");
          lcd.print(batteryEndVolts);
          updateDisplay = 0;
          batteryProg = 2;
        } else if (cursorCount == 2) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "Li2TiO3 (LTO)";
          batteryNom = 2.40;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = 2.85v");
          updateDisplay = 0;
          batteryProg = 3;
          Serial.println("LTO");
        } else if (cursorCount == 3) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "LiCoO2 (LCO)";
          batteryNom = 3.60;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = 4.2v");
          updateDisplay = 0;
          batteryProg = 4;
        } else if (cursorCount == 4) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "LiMn2O4 (LMO)";
          batteryNom = 3.80;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = 4.2v");
          updateDisplay = 0;
          batteryProg = 5;
        } else if (cursorCount == 5) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "LiNiMnCoO2 (NMC)";
          batteryNom = 3.70;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = 4.2v");
          updateDisplay = 0;
          batteryProg = 6;
        } else if (cursorCount == 6) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "LiNiCoAlO2 (NCA)";
          batteryNom = 3.60;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Default end = 4.2v");
          updateDisplay = 0;
          batteryProg = 7;
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
            setLCDPointer(0);
            updateDisplay = 0;
            batteryRun = 1;
          } else if (cursorCount == 1) {
            setLCDPointer(1);
            updateDisplay = 0;
            batteryRun = 2;
          } else if (cursorCount == 2) {
            setLCDPointer(2);
            updateDisplay = 0;
            batteryRun = 3;
          } else if (cursorCount == 3) {
            setLCDPointer(3);
            updateDisplay = 0;
            batteryRun = 4;
          } else {
            cursorCount = 0;
            updateDisplay = 1;
          }
          checkButtons();
          if (enterB == 1) {
            enterB = 0;
            cursorCount = 0;
            updateDisplay = 1;
            chargeLiBattery(batteryRun, batteryName, batteryCurrent, batteryEndVolts, safetyTimer, batteryCutoff);
            return;
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
