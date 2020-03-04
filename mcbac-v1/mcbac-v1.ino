// Created by: WestleyR
// Email(s): westleyr@nym.hush.com
// Last modifyed date: Mar 4, 2020
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
#include <stdarg.h>
#include "LiquidCrystal_I2C.h"

#define MCBAC_VERSION "v1.0.0-beta-1"
#define MCBAC_DATE "March 4, 2020"

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

  // Rotary encoder
  attachInterrupt(0, cursorCW, RISING);
  attachInterrupt(1, cursorCC, RISING);

  // For debugging
  Serial.begin(9600);

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MCBAC ");
  lcd.print(MCBAC_VERSION);
  lcd.setCursor(0, 1);
  lcd.print(MCBAC_DATE);
  delay(250);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Copyright WestleyR");
  lcd.setCursor(3, 1);
  lcd.print("This divice is");
  lcd.setCursor(1, 2);
  lcd.print("licensed under The");
  lcd.setCursor(1, 3);
  lcd.print("Clear BSD License.");
  delay(350);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("https://github.com/");
  lcd.setCursor(3, 1);
  lcd.print("WestleyR/mcbac");
  delay(250);
  lcd.clear();
  lcd.setCursor(0, 0);
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
    delay(150);
    return 1;
  } else if (digitalRead(buttonBack) == LOW) {
    backB = 1;
    delay(150);
    return 2;
  } else if (digitalRead(buttonUp) == LOW) {
    upB = 1;
    delay(150);
    return 3;
  } else if (digitalRead(buttonDown) == LOW) {
    downB = 1;
    delay(150);
    return 4;
  }
  // Maybe dont do this:
  resetAllButtons();
  return 0;
}

// The function that charges a battery
void chargeLi() {
  lcd.clear();
  lcd.setCursor(0, 0);
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
int batteryType;

void chargeCVBattery() {
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("C: ");
      lcd.print(batteryName);
      lcd.setCursor(0, 1);
      lcd.print("V ");
      lcd.print(00.00);
      lcd.print("   CV ");
      lcd.print(batteryEndVolts);
      lcd.setCursor(0, 2);
      lcd.print("I ");
      lcd.print(0000);
      lcd.print("  IL ");
      lcd.print(batteryCurrent);
      lcd.setCursor(0, 3);
      lcd.print("T ");
      lcd.print(000);
      lcd.print("  CT ");
      lcd.print(safetyTimer);
      updateDisplay = 0;
    }
    checkButtons();
  }
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("Done charging");
  lcd.setCursor(0, 1);
  lcd.print("mah na");

  delay(200);
  while (digitalRead(buttonBack) == HIGH) {
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      break;
    }
  }

  updateDisplay = 1;
}

void dischargeCC(String batteryName, int batteryCurrent, float batteryEndVolts, int batteryCutoff, int safetyTimer) {
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("D: ");
      lcd.print(batteryName);
      lcd.setCursor(0, 1);
      lcd.print("V ");
      lcd.print(00.00);
      lcd.print("  EndV ");
      lcd.print(batteryEndVolts);
      lcd.setCursor(0, 2);
      lcd.print("I: ");
      lcd.print(0000);
      lcd.print("  Iset ");
      lcd.print(batteryCurrent);
      lcd.setCursor(0, 3);
      lcd.print("T ");
      lcd.print(000);
      lcd.print("  Tcut ");
      lcd.print(safetyTimer);
      updateDisplay = 0;
    }
    checkButtons();
    //if (enterB == 1) {
    //  enterB = 0;
    //  //break;
    //}
  }
  //lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("Done: ");
  lcd.print(batteryName);

  delay(200);
  while (digitalRead(buttonBack) == HIGH) {
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      break;
    }
  }

  updateDisplay = 1;
}

// HERE
//void chargeLiBattery(int batteryRun, int batteryType, String batteryName, int batteryCurrent, float batteryEndVolts, int safetyTimer, int batteryCutoff, bool ccCharge) {
void chargeLiBattery() {
  Serial.print("BatteryType: ");
  Serial.println(batteryType);
  Serial.print("BatteryRun: ");
  Serial.println(batteryRun);
  if (batteryType == 1) { // Lithium type
    if (batteryRun == 1) {
      // Charge
      if (setCVFunc() != 0) {
        cursorCount = 0;
        updateDisplay = 1;
        return;
      }
      //chargeCVBattery(batteryName, batteryCurrent, batteryEndVolts, batteryCutoff, safetyTimer);
      chargeCVBattery();

    } else if (batteryRun == 2) {
      // Discharge
      if (setCCDFunc() != 0) {
        return;
        cursorCount = 0;
        updateDisplay = 1;
      }
      dischargeCC(batteryName, batteryCurrent, batteryEndVolts, batteryCutoff, safetyTimer);

    } else if (batteryRun == 3) {
      // Cycle test
    } else if (batteryRun == 4) {
      // SoC test
    }
  } else if (batteryType == 2) { // Nickel type
    if (batteryRun == 1) {
      // Charge
      if (setCCFunc() != 0) {
        cursorCount = 0;
        updateDisplay = 1;
        return;
      }
      //chargeCVBattery(batteryName, batteryCurrent, batteryEndVolts, batteryCutoff, safetyTimer);
      chargeCVBattery();

    } else if (batteryRun == 2) {
      // Discharge
      if (setCCDFunc() != 0) {
        return;
        cursorCount = 0;
        updateDisplay = 1;
      }
      dischargeCC(batteryName, batteryCurrent, batteryEndVolts, batteryCutoff, safetyTimer);

    } else if (batteryRun == 3) {
      // Cycle test
    } else if (batteryRun == 4) {
      // SoC test
    }
  }


  updateDisplay = 1;
  cursorCount = 0;
}

int changeCurrent(int line, int before) {
  int ret;

  cursorCount = before / 10;
  lcd.setCursor(16, line);
  lcd.blink();
  // Wait for the user to set the charge current
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.setCursor(12, line);
      lcd.print("        ");
      lcd.setCursor(13, line);
      lcd.print(cursorCount * 10);
      updateDisplay = 0;
    }
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      ret = cursorCount * 10;
      break;
    }
  }
  lcd.noBlink();

  return ret;
}

float changeVolts(int line, float before) {
  float ret = before;

  cursorCount = 128;
  int lastCount = cursorCount;
  lcd.setCursor(16, line);
  lcd.blink();
  // Wait for the user to set the end voltage
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      if (lastCount > cursorCount) {
        ret -= 0.01;
        lastCount = cursorCount;
      } else if (lastCount < cursorCount) {
        ret += 0.01;
        lastCount = cursorCount;
      }
      lcd.setCursor(12, line);
      lcd.print("        ");
      lcd.setCursor(14, line);
      lcd.print(ret);
      Serial.println(ret);
      updateDisplay = 0;
    }
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      break;
    }
  }
  lcd.noBlink();

  return ret;
}


int changeCutoff(int line, int before) {
  int ret;
  cursorCount = before;
  lcd.setCursor(16, line);
  lcd.blink();
  // Wait for the user to set the end voltage
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.setCursor(12, line);
      lcd.print("        ");
      lcd.setCursor(14, line);
      lcd.print(cursorCount);
      updateDisplay = 0;
    }
    checkButtons();
    if (enterB == 1) {
      ret = cursorCount;
      enterB = 0;
      break;
    }
  }
  lcd.noBlink();

  return ret;
}

int changeTimer(int line, int before) {
  int ret;
  cursorCount = before;
  lcd.setCursor(16, line);
  lcd.blink();
  // Wait for the user to set the end voltage
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.setCursor(14, line);
      lcd.print("      ");
      lcd.setCursor(16, line);
      lcd.print(cursorCount);
      updateDisplay = 0;
    }
    checkButtons();
    if (enterB == 1) {
      ret = cursorCount;
      enterB = 0;
      break;
    }
  }
  lcd.noBlink();

  return ret;
}

bool changePulse(int line, bool before) {
  bool ret;
  if (before == true) {
    cursorCount = 0;
  } else {
    cursorCount = 0;
  }

  lcd.setCursor(16, line);
  lcd.blink();
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      lcd.setCursor(14, line);
      lcd.print("      ");
      lcd.setCursor(16, line);
      if(((cursorCount / 2) & 1) == 0) {
        lcd.print("true");
      } else {
        lcd.print("false");
      }
      updateDisplay = 0;
    }
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      break;
    }
  }
  lcd.noBlink();

  return ret;
}

int setCVFunc() {
  int batteryAdjust;

  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("Current MA ");
  lcd.print(batteryCurrent);

  lcd.setCursor(2, 1);
  lcd.print("End volts ");
  lcd.print(batteryEndVolts);

  lcd.setCursor(2, 2);
  lcd.print("Cutoff MA ");
  lcd.print(batteryCutoff);

  lcd.setCursor(2, 3);
  lcd.print("Safety timer ");
  lcd.print(safetyTimer);

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
      cursorCount = 4;
      //updateDisplay = 1;
    }
    checkButtons();
    if (backB == 1) break; // return 1 as user pushed the back button
    if (enterB == 1) {
      int lastCursorState = cursorCount;
      enterB = 0;
      if (batteryAdjust == 1) {
        batteryCurrent = changeCurrent(0, batteryCurrent);
        batteryAdjust = 0;
      } else if (batteryAdjust == 2) {
        batteryEndVolts = changeVolts(1, batteryEndVolts);
        batteryAdjust = 0;
      } else if (batteryAdjust == 3) {
        batteryCutoff = changeCutoff(2, batteryCutoff);
        batteryAdjust = 0;
      } else if (batteryAdjust == 4) {
        safetyTimer = changeTimer(3, safetyTimer);
        batteryAdjust = 0;
      } else if (batteryAdjust == 5) {
        enterB = 0;
        updateDisplay = 1;
        cursorCount = 0;
        // User said to start the function
        return 0;
      }
      cursorCount = lastCursorState;
    }
  }
  updateDisplay = 1;

  return 1;
}

float batteryVoltLimit;
bool batteryPulseCharge = true;

int setCCFunc() {
  int batteryAdjust;
  int updateScroll = 1;
  int page = 1;

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
      if (page == 2) {
        page = 1;
        updateScroll = 1;
      }
    } else if (cursorCount == 4) {
      if (page == 1) {
        page = 2;
        updateScroll = 2;
      }
      setLCDPointer(0);
      updateDisplay = 0;
      batteryAdjust = 5;
    } else if (cursorCount == 5) {
      setLCDPointer(1);
      updateDisplay = 0;
      batteryAdjust = 6;
    } else if (cursorCount == 6) {
      lcd.setCursor(0, 0);
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print("G ");
      lcd.setCursor(0, 2);
      lcd.print("O ");
      lcd.setCursor(0, 3);
      lcd.print("  ");
      updateDisplay = 0;
      batteryAdjust = 7;
    } else {
      cursorCount = 6;
    }

    if (updateScroll == 1) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Current MA ");
      lcd.print(batteryCurrent);

      lcd.setCursor(2, 1);
      lcd.print("Volt limit ");
      lcd.print(batteryVoltLimit);

      lcd.setCursor(2, 2);
      lcd.print("Delta V ");
      lcd.print(batteryEndVolts);

      lcd.setCursor(2, 3);
      lcd.print("Cutoff MAH ");
      lcd.print(batteryCutoff);
      updateScroll = 0;
    } else if (updateScroll == 2) {
      // Shift the display down
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Safety timer ");
      lcd.print(safetyTimer);

      lcd.setCursor(2, 1);
      lcd.print("Pulse ");
      lcd.print(batteryPulseCharge ? "true" : "false");
      updateScroll = 0;
    }

    checkButtons();
    if (backB == 1) break; // return 1 as user pushed the back button
    if (enterB == 1) {
      int lastCursorState = cursorCount;
      enterB = 0;
      if (batteryAdjust == 1) {
        batteryCurrent = changeCurrent(0, batteryCurrent);
        batteryAdjust = 0;
      } else if (batteryAdjust == 2) {
        batteryVoltLimit = changeVolts(1, batteryVoltLimit);
        batteryAdjust = 0;
      } else if (batteryAdjust == 3) {
        batteryEndVolts = changeVolts(2, batteryEndVolts);
        batteryAdjust = 0;
      } else if (batteryAdjust == 4) {
        batteryCutoff = changeCutoff(3, batteryCutoff);
        batteryAdjust = 0;
      } else if (batteryAdjust == 5) {
        safetyTimer = changeTimer(0, safetyTimer);
        batteryAdjust = 0;
      } else if (batteryAdjust == 6) {
        batteryPulseCharge = changePulse(1, batteryPulseCharge);
        batteryAdjust = 0;
      } else if (batteryAdjust == 7) {
        enterB = 0;
        updateDisplay = 1;
        cursorCount = 0;
        // User said to start the function
        return 0;
      }
      cursorCount = lastCursorState;
    }
  }
  updateDisplay = 1;

  return 1;
}

int setCCDFunc() {

  return 1;
}

int selectFunction(int num, const char* list, ...) {
  int batteryRun = -1;

  va_list nameValues;
  va_start(nameValues, list);

  int lineCount = 0;

  lcd.clear();

  // Print the options
  for (int i = 0; i < num; i++) {
    lcd.setCursor(2, lineCount);
    lcd.print(list);
    lineCount++;
    list = va_arg(nameValues, char*);
  }
  va_end(nameValues);

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
      cursorCount = 4;
    }
    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      cursorCount = 0;
      updateDisplay = 1;
      break;
    }
  }

  return batteryRun;
}


const char* CHARGE = "Charge";
const char* DISCHARGE = "Discharge";
const char* CYCLE = "Cycle test";
const char* SOCTEST = "SoC test";


void configCharge() {
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
          cursorCount = 6;
          //updateDisplay = 1;
        }
      }
      checkButtons();

      if (enterB == 1) {
        enterB = 0;
        cursorCount = 0;
        updateDisplay = 0;
        // Ask for charge, discharge, or cycle test
        batteryRun = selectFunction(4, CHARGE, DISCHARGE, CYCLE, SOCTEST);
        if (batteryRun == -1) {
          cursorCount = 0;
          updateDisplay = 1;
          return;
        }
        Serial.println("SELECTIED BATTERY: ");
        Serial.print("batteryType: ");
        Serial.println(batteryType);
        Serial.print("batteryProg: ");
        Serial.println(batteryProg);
        chargeLiBattery();
        //        if (batteryRun == 1) { // Charge
        //          //chargeLiBattery(batteryRun, batteryType, batteryName, batteryCurrent, batteryEndVolts, safetyTimer, batteryCutoff, false);
        //          chargeLiBattery();
        //          cursorCount = 0;
        //          return;
        //        } else if (batteryRun == 2) { // Discharge
        //          batteryEndVolts = batteryNom;
        //          //chargeLiBattery(2, batteryType, batteryName, batteryCurrent, batteryNom, safetyTimer, batteryCutoff, false);
        //          chargeLiBattery();
        //          cursorCount = 0;
        //          return;
        //        }
      }
    }
  } else if (batteryType == 2) {
    // Nickel battery
    while (digitalRead(buttonBack) == HIGH) {
      if (updateDisplay == 1) {
        if (cursorCount == 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "Nimh";
          batteryNom = 1.2;
          batteryEndVolts = 5;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Delta V = ");
          lcd.print(batteryEndVolts);
          updateDisplay = 0;
          batteryProg = 1;
        } else if (cursorCount == 1) {
          lcd.clear();
          lcd.setCursor(0, 0);
          batteryName = "Nicd";
          batteryNom = 1.2;
          batteryEndVolts = 5;
          lcd.print(batteryName);
          lcd.setCursor(0, 2);
          lcd.print("Noninal = ");
          lcd.print(batteryNom);
          lcd.setCursor(0, 3);
          lcd.print("Delta V = ");
          lcd.print(batteryEndVolts);
          updateDisplay = 0;
          batteryProg = 2;
        } else {
          cursorCount = 2;
          //updateDisplay = 1;
        }
      }
      checkButtons();

      if (enterB == 1) {
        enterB = 0;
        cursorCount = 0;
        updateDisplay = 0;
        // Ask for charge, discharge, or cycle test
        batteryRun = selectFunction(4, CHARGE, DISCHARGE, CYCLE, SOCTEST);
        if (batteryRun == -1) {
          cursorCount = 0;
          updateDisplay = 1;
          return;
        }

        if (batteryRun == 1) { // Charge
          //chargeLiBattery(batteryRun, batteryType, batteryName, batteryCurrent, batteryEndVolts, safetyTimer, batteryCutoff, true);
          chargeLiBattery();
          cursorCount = 0;
          updateDisplay = 1;
          return;
        } else if (batteryRun == 2) { // Discharge
          batteryEndVolts = batteryNom;
          //chargeLiBattery(2, batteryType, batteryName, batteryCurrent, batteryNom, safetyTimer, batteryCutoff, false);
          chargeLiBattery();
          cursorCount = 0;
          updateDisplay = 1;
          return;
        }
        return;
      }
    }
    cursorCount = 0;
    updateDisplay = 1;
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
      lcd.setCursor(2, 0);
      lcd.print("Select a battery");
      lcd.setCursor(5, 2);
      lcd.print("Lithium");
      lcd.setCursor(5, 3);
      lcd.print("batteries");
      batteryType = 1;
      updateDisplay = 0;
      Serial.println("Lithium battery");
    } else if (cursorCount == 1) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Select a battery");
      lcd.setCursor(5, 2);
      lcd.print("Nickel");
      lcd.setCursor(5, 3);
      lcd.print("batteries");
      batteryType = 2;
      Serial.println("Nickel battery");
      updateDisplay = 0;
    } else if (cursorCount == 2) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Select a battery");
      lcd.setCursor(5, 2);
      lcd.print("Lead acid");
      lcd.setCursor(5, 3);
      lcd.print("batteries");
      batteryType = 3;
      Serial.println("Lead acid battery");
      updateDisplay = 0;
    } else if (cursorCount == 3) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Other functions");
      lcd.setCursor(7, 2);
      lcd.print("Power");
      lcd.setCursor(7, 3);
      lcd.print("Supply");
      batteryType = 4;
      Serial.println("Power Supply");
      updateDisplay = 0;
    } else if (cursorCount == 4) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Other functions");
      lcd.setCursor(7, 2);
      lcd.print("DC Load");
      batteryType = 5;
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
    configCharge();
  }
}

void loop() {
  checkButtons();
  mainMenu();
}

// vim: tabstop=2 shiftwidth=2 expandtab autoindent softtabstop=0
