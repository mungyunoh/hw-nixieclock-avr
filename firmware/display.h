#ifndef DISPLAY_H
#define DISPLAY_H

#include "global.h"

void displayInit(void);
void displayTime(time_t time);
void displayShow();

uint8_t displayHighVoltageRead();
void displayHighVoltageEnable();
void displayHighVoltageDisable();

#endif