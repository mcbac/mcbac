// Created by: WestleyR
// Email: westleyr@nym.hush.com
// Url: https://github.com/WestleyR/mcbac
// Last modified date: 2020-05-11
//
// This file is licensed under the terms of
//
// The Clear BSD License
//
// Copyright (c) 2020 WestleyR
// All rights reserved.
//
// This software is licensed under a Clear BSD License.
//

#include <Wire.h>

#include "FaBoLCD_PCF8574.h"
#include "Adafruit_MCP4725.h"
#include "Adafruit_ADS1015.h"

#define MCBAC_VERSION "v1.0.0-beta-4"
#define MCBAC_DATE "May 11, 2020"

#define buttonEnter 4
#define buttonBack 5
#define buttonUp 6
#define buttonDown 7

#define buzzer 8
#define battery_relay 9
#define charge_discharge_relay A1
#define CC_CV A3

#define R_CW 2
#define R_CC 3

// 20x4 LCD with I2C adaptor
FaBoLCD_PCF8574 lcd;

Adafruit_MCP4725 dacVout;
Adafruit_MCP4725 dacIout;
Adafruit_ADS1115 adc;

void setup() {
  pinMode(buttonEnter, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);
  pinMode(battery_relay, OUTPUT);
  pinMode(charge_discharge_relay, OUTPUT);
  pinMode(CC_CV, INPUT);

  digitalWrite(buzzer, LOW);
  digitalWrite(battery_relay, LOW);
  digitalWrite(charge_discharge_relay, LOW);

  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  pinMode(R_CW, INPUT_PULLUP);
  pinMode(R_CC, INPUT_PULLUP);

  dacVout.begin(0x63);

  // Make sure the output is zero
  setVOut(0);

  dacIout.begin(0x62);

  // Make sure the output is zero
  setIOut(0);

  adc.begin();

  // Initialize the LCD
  lcd.begin(20, 4);

  // Rotary encoder
  attachInterrupt(0, cursorCW, RISING);
  attachInterrupt(1, cursorCC, RISING);

  // For debugging/logging
  Serial.begin(9600);

  // Print version, and info
  //lcd.backlight();
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
  delay(250);
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
    // TODO: changed to -- untill fixed wires
    cursorCount--;
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
    // TODO: changed to ++ untill fixed wires
    cursorCount++;
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
  resetAllButtons();
  return 0;
}

int outputVOffset = 0;

// volts can be 0.00-15.00
void setVOut(float volts) {
  // 4096 is the max value

  int v = volts * 273;
  v += outputVOffset;
  if (v < 0) v = 0;
  if (v > 4096) v = 4096;
  dacVout.setVoltage(v, false);
}

// TODO: Fix this to real values
const static int CURRENT_OFFSET = 0;
const static float CURRENT_CAL = 0.65;

int outputIOffset = 0;

// Current can be from 0-2000;
void setIOut(int ma) {
  // 4096 is the max value

  // TODO: FIXME:
  //int i = (ma + CURRENT_OFFSET) / CURRENT_CAL;
  int i = 520;
  i += ma * 4.25;
  i += outputIOffset;
  dacIout.setVoltage(i, false);
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

// Battery values
int batteryRun;
int batteryCurrent = 100;
int safetyTimer = 120;
int batteryCutoff = 100;
int batteryCutoffMAH = 5000;
int batteryType;
float batteryNom;
float batteryEndVolts;
float batteryVoltLimit = 4.00;
float batteryDeltaV = 5.00;
String batteryName;
bool batteryPulseCharge = true;

void setDecadeValue(float *value, float inc, bool dec, int updateValue, int cur, int line) {
  cursorCount = 128;
  int lastCount = cursorCount;
  int decadeCount = 100;
  int blinkCursor;
  updateDisplay = 1;
  lcd.setCursor(cur, line);
  lcd.cursor();
  // Wait for the user to set the end voltage
  while (digitalRead(buttonBack) == HIGH) {
    if (updateDisplay == 1) {
      if (lastCount > cursorCount) {
        *value -= (inc * decadeCount);
        lastCount = cursorCount;
        if (updateValue == 1) {
          setVOut(*value);
        } else if (updateValue == 2) {
          //setIOut((int)*value);
          batteryCurrent = (int) * value;
        }
        updateDisplay = 0;
      } else if (lastCount < cursorCount) {
        *value += (inc * decadeCount);
        lastCount = cursorCount;
        if (updateValue == 1) {
          setVOut(*value);
        } else if (updateValue == 2) {
          //setIOut((int)*value);
          batteryCurrent = (int) * value;
        }
        updateDisplay = 0;
      }

      lcd.setCursor(cur, line);
      if (dec) {
        lcd.print(*value);
      } else {
        lcd.print((int)*value);
        if (*value == 0) lcd.print("  ");
      }
      lcd.print(" ");
      if (decadeCount == 100) {
        blinkCursor = cur;
      } else if (decadeCount == 10) {
        blinkCursor = cur + 1;
        if (dec) blinkCursor++;
      } else if (decadeCount == 1) {
        blinkCursor = cur + 2;
        if (dec) blinkCursor++;
      }
      if (dec) {
        if (*value >= 10) blinkCursor++;
      } else {
        if (*value >= 1000) blinkCursor++;
      }
      lcd.setCursor(blinkCursor, line);

      updateDisplay = 0;
    }

    // Update the volts if in power supply mode
    if (updateValue == 1 || updateValue == 2) readVolts(blinkCursor, line);

    checkButtons();
    if (enterB == 1) {
      enterB = 0;
      decadeCount /= 10;
      updateDisplay = 1;
      if (decadeCount < 1) break;
    }
  }
  lcd.noCursor();
  delay(250);
}

float batteryMWH;
unsigned long lastUpdated = millis();

#define BETWEEN(value, min, max) (value < max && value > min)
#define CV true
#define CC false
#define MAX_OFFSET 50

bool vcal = false;

// FixOutputVolts will slightly adjust the real volts to match the set volts
void fixOutputVolts(bool supplyMode, float real, float set) {
  // Only fix the output if in CV mode
  if (supplyMode == CV) {
    if (real < set) {
      outputVOffset += 1;
    } else if (real > set) {
      outputVOffset -= 1;
    }
    if (outputVOffset > MAX_OFFSET) outputVOffset = MAX_OFFSET;
    if (outputVOffset < -MAX_OFFSET) outputVOffset = -MAX_OFFSET;
    setVOut(batteryEndVolts);
    Serial.print("outputoffset: ");
    Serial.println(outputVOffset);
  }
}

void fixOutputCurrent(bool supplyMode, float real, float set) {
  // Only fix the output if in CC mode
  if (supplyMode == CC) {
    if (real < set) {
      outputIOffset += 1;
    } else if (real > set) {
      outputIOffset -= 1;
    }
    if (outputIOffset > MAX_OFFSET) outputIOffset = MAX_OFFSET;
    if (outputIOffset < -MAX_OFFSET) outputIOffset = -MAX_OFFSET;
    setIOut(batteryCurrent);
  }
}

float i;
float v;
bool supplyMode;

// readVolts will read the volts, update the display, and do small adjustments to the output
// if needed. Used in Power Supply mode.
void readVolts(int row, int line) {
  // Read the volts
  v = adc.readADC_SingleEnded(0);
  // Calabrate the voltage
  v = v / 1750;

  // Now read the current
  i = adc.readADC_SingleEnded(1);
  // Remove the diff opamp offset, then the calabrate value
  i = (i - 3561) / 25.4;
  if (i < 0) {
    i = 0;
  }

  // Only update every 0.5s
  if (millis() - lastUpdated > 500) {
    // Print the volts
    lcd.setCursor(4, 1);
    lcd.print(v);
    lcd.print(" ");

    // Print the current
    lcd.setCursor(4, 2);
    lcd.print((int)i);
    lcd.print("   ");

    // Print the Watts
    lcd.setCursor(4, 3);
    float w = (i / 1000) * v;
    if (w < 0) {
      lcd.print("0.00");
      w = 0;
    } else {
      lcd.print(w);
    }
    lcd.print(" ");

    // Print the mWH
    unsigned long t = millis() - lastUpdated;
    float mwh = (t * w) / 36;
    batteryMWH += mwh;

    lcd.setCursor(14, 3);
    lcd.print((int)(batteryMWH / 100));
  
    lastUpdated = millis();
  }

  // Fix the output error only if current is 0
  if (i == 0) fixOutputVolts(CV, v, batteryEndVolts);

  lcd.setCursor(18, 0);
  if (((batteryEndVolts - v) >= 0.02) || ((batteryEndVolts - v) <= -0.02)) {
    lcd.print("CC");
    fixOutputCurrent(CC, i, batteryCurrent);
  } else {
    lcd.print("CV");
  }

  if (row != -1 && line != -1) lcd.setCursor(row, line);
}

void powerSupply() {
  int batteryAdjust;
  int updateOutput = 1;
  cursorCount = 0;

  digitalWrite(battery_relay, HIGH);
  digitalWrite(charge_discharge_relay, LOW);

  while (digitalRead(buttonBack) == HIGH) {
    if (updateOutput == 1) {
      setVOut(batteryEndVolts);
      setIOut(batteryCurrent);
    }
    if (updateDisplay == 1) {
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Power Supply");
      lcd.setCursor(0, 1);
      lcd.print("  V ");
      lcd.setCursor(10, 1);
      lcd.print("SET ");
      lcd.print(batteryEndVolts);
      lcd.setCursor(0, 2);
      lcd.print("  I ");
      lcd.setCursor(10, 2);
      lcd.print("SET ");
      lcd.print(batteryCurrent);
      lcd.setCursor(0, 3);
      lcd.print("  W ");
      lcd.setCursor(10, 3);
      lcd.print("mWH ");
      updateDisplay = 0;
    }

    if (cursorCount == 0) {
      setLCDPointer(1);
      updateDisplay = 0;
      batteryAdjust = 1;
    } else if (cursorCount == 1) {
      setLCDPointer(2);
      updateDisplay = 0;
      batteryAdjust = 2;
    } else {
      cursorCount = 1;
    }
    readVolts(-1, -1);
    checkButtons();
    if (enterB == 1) {
      int lastCursorState = cursorCount;
      enterB = 0;
      if (batteryAdjust == 1) { // Volts
        setDecadeValue(&batteryEndVolts, 0.01, true, 1, 14, 1);
        batteryAdjust = 0;
        updateOutput = 1;
      } else if (batteryAdjust == 2) { // Current
        float c = float(batteryCurrent);
        setDecadeValue(&c, 1, false, 2, 14, 2);
        batteryCurrent = c;
        batteryAdjust = 0;
        updateOutput = 1;
      }
      cursorCount = lastCursorState;
    }
  }

  digitalWrite(battery_relay, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("End Power Supply");
  lcd.setCursor(0, 1);
  lcd.print("Total mWH: ");
  lcd.setCursor(0, 2);
  lcd.print(batteryMWH);

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

void chargeLiBattery() {
  if (batteryType == 1) { // Lithium type
    if (batteryRun == 1) {
      // Charge
      if (setCVFunc() != 0) {
        cursorCount = 0;
        updateDisplay = 1;
        return;
      }
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
  } else if (batteryType == 3) {
    // Lead acid
  } else if (batteryType == 4) { // Power supply
    powerSupply();

  } else if (batteryType == 5) {
    // DC load
  }


  updateDisplay = 1;
  cursorCount = 0;
}

int batteryMAH = 100;

int setCVFunc() {
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
      lcd.print("Battery mAH ");
      lcd.print(batteryMAH);

      lcd.setCursor(2, 1);
      lcd.print("Charge ");
      lcd.print(batteryCurrent);
      lcd.print(" C");
      lcd.print((float)((float)batteryCurrent / (float)batteryMAH));

      lcd.setCursor(2, 2);
      lcd.print("End volts ");
      lcd.print(batteryEndVolts);

      lcd.setCursor(2, 3);
      lcd.print("Cutoff MA ");
      lcd.print(batteryCutoff);
      updateScroll = 0;
    } else if (updateScroll == 2) {
      // Shift the display down
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Safety timer ");
      lcd.print(safetyTimer);

      lcd.setCursor(2, 1);
      lcd.print("Cutoff MAH  ");
      lcd.print(batteryCutoffMAH);
      updateScroll = 0;
    }

    checkButtons();
    if (backB == 1) break; // return 1 as user pushed the back button
    if (enterB == 1) {
      int lastCursorState = cursorCount;
      enterB = 0;
      if (batteryAdjust == 1) { // Battery mAH
        float c = batteryMAH;
        setDecadeValue(&c, 1, false, 0, 14, 0);
        batteryMAH = c;

        // Charge the battery at 1C (by default), if under 2Amps
        batteryCurrent = batteryMAH * 1; // The default charge rate
        if (batteryCurrent > 2000) batteryCurrent = 2000;
        if (page == 1) updateScroll = 1;

        // 110% of the battery capacity for the mAH cutoff
        batteryCutoffMAH = batteryMAH * 1.1;

        batteryAdjust = 0;
      } else if (batteryAdjust == 2) { // Charge mA
        float c = batteryCurrent;
        setDecadeValue(&c, 1, false, 0, 9, 1);
        batteryCurrent = c;
        if (page == 1) updateScroll = 1;
        batteryAdjust = 0;
      } else if (batteryAdjust == 3) { // End V
        setDecadeValue(&batteryEndVolts, 0.01, true, 0, 12, 2);
        batteryAdjust = 0;
      } else if (batteryAdjust == 4) { // Cutoff MAH
        float c = batteryCutoff;
        setDecadeValue(&c, 1, false, 0, 12, 3);
        batteryCutoff = c;
        batteryAdjust = 0;
      } else if (batteryAdjust == 5) { // Safety timer
        float t = safetyTimer;
        setDecadeValue(&t, 1, false, 0, 15, 0);
        safetyTimer = t;
        batteryAdjust = 0;
      } else if (batteryAdjust == 6) { // Cutoff mAH
        float c = batteryCutoffMAH;
        setDecadeValue(&c, 1, false, 0, 14, 1);
        batteryCutoffMAH = c;
        batteryAdjust = 0;
      } else if (batteryAdjust == 7) { // GO
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

int setCCFunc() {
  /*
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
        lcd.print(batteryCutoffMAH);
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
        if (batteryAdjust == 1) { // Current MA
          batteryCurrent = changeInt10(0, 10, batteryCurrent);
          batteryAdjust = 0;
        } else if (batteryAdjust == 2) { // Volt limit
          batteryVoltLimit = changefloatHalf(1, 10, batteryVoltLimit);
          batteryAdjust = 0;
        } else if (batteryAdjust == 3) { // Delta V
          batteryDeltaV = changeFloat05(2, 7, batteryDeltaV);
          batteryAdjust = 0;
        } else if (batteryAdjust == 4) { // Cutoff MAH
          batteryCutoffMAH = changeInt10(3, 10, batteryCutoffMAH);
          batteryAdjust = 0;
        } else if (batteryAdjust == 5) { // Safety timer
          safetyTimer = changeInt10(0, 12, safetyTimer);
          batteryAdjust = 0;
        } else if (batteryAdjust == 6) { // Pulse
          batteryPulseCharge = changeBool(1, 5, batteryPulseCharge);
          batteryAdjust = 0;
        } else if (batteryAdjust == 7) { // GO
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

  */
  return 1;
}

// The Constent current discharge setup
int setCCDFunc() {
  /*int batteryAdjust;
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
      lcd.print("Cutoff V ");
      lcd.print(batteryEndVolts);

      lcd.setCursor(2, 2);
      lcd.print("Cutoff MAH ");
      lcd.print(batteryCutoff);

      lcd.setCursor(2, 3);
      lcd.print("Safety timer ");
      lcd.print(safetyTimer);
      updateScroll = 0;
    }

    checkButtons();
    if (backB == 1) break; // return 1 as user pushed the back button
    if (enterB == 1) {
      int lastCursorState = cursorCount;
      enterB = 0;
      if (batteryAdjust == 1) { // Current MA
        batteryCurrent = changeInt10(0, 10, batteryCurrent);
        batteryAdjust = 0;
      } else if (batteryAdjust == 2) { // Cutoff V
        batteryEndVolts = changeFloat(1, 8, batteryEndVolts);
        batteryAdjust = 0;
      } else if (batteryAdjust == 3) { // Cutoff MAH
        batteryCutoffMAH = changeInt10(2, 10, batteryCutoffMAH);
        batteryAdjust = 0;
      } else if (batteryAdjust == 4) { // Safety timer
        safetyTimer = changeInt10(3, 12, safetyTimer);
        batteryAdjust = 0;
      } else if (batteryAdjust == 7) { // GO
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
  */
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

// TODO: this may not be needed
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
        chargeLiBattery();
      }
    }
    cursorCount = 0;
    updateDisplay = 1;
    return;
  } else if (batteryType == 2) {
    // Nickel battery
    while (digitalRead(buttonBack) == HIGH) {
      if (updateDisplay == 1) {
        if (cursorCount == 0) {
          lcd.clear();
          lcd.setCursor(8, 0);
          batteryName = "NiMH";
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
          lcd.setCursor(8, 0);
          batteryName = "NiCD";
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
        chargeLiBattery();
      }
    }
    cursorCount = 0;
    updateDisplay = 1;
    return;
  } else if (batteryType == 3) {
    // Lead Acid battery
    cursorCount = 0;
    updateDisplay = 1;
    return;
  } else if (batteryType == 4) {
    // Power supply mode
    chargeLiBattery();

    cursorCount = 0;
    updateDisplay = 1;
    return;
  } else if (batteryType == 5) {
    // DC load mode
    cursorCount = 0;
    updateDisplay = 1;
    return;
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
