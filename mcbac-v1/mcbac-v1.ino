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


LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  pinMode(buttonEnter, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  
  // Initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("MCBAC V1.0.0");
  delay(3000);
}

// When 1; update the LCD
int updateDisplay = 1;

int curser;
int curserRow;

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

int checkButtons() {
  if (digitalRead(buttonEnter) == LOW) {
    enterB = 1;
    return 1;
  } else if (digitalRead(buttonBack) == LOW) {
    backB = 1;
    return 2;
  } else if (digitalRead(buttonUp) == LOW) {
    upB = 1;
    return 3;
  } else if (digitalRead(buttonDown) == LOW) {
    downB = 1;
    return 4;
  }
  // Maybe dont do this:
  resetAllButtons();
  return 0;
}

void curserUp() {  
  curser++;
  updateDisplay = 1;
}

void curserDown() {  
  curser--;
  updateDisplay = 1;
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

// To remember the battery program
// 1 = lipo
// 2 = ninh
// ...
int batteryProg;

void mainMenu() {
  if (updateDisplay == 1) {
    if (curser == 0) {
      lcd.clear();
      lcd.setCursor(3, 2);
      lcd.print("Lithium");
      lcd.setCursor(3, 2);
      lcd.print("batteries");
      batteryProg = 1;
   } else if (curser == 1) {
      lcd.clear();
      lcd.setCursor(3, 3);
      lcd.print("Nickel");
      lcd.setCursor(3, 3);
      lcd.print("batteries");
      batteryProg = 2;
    } else {
      curser = 0;
      updateDisplay = 1;
    }
    updateDisplay = 0;
  }

  if (enterB == 1) {
    enterB = 0;
    chargeBattery(batteryProg);
  }
}

void loop() {
  checkButtons();
  mainMenu();
}

// vim: tabstop=2 shiftwidth=2 expandtab autoindent softtabstop=0
