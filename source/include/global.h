#pragma once

#include "debug.h"

#include "Charger.h"
#include "Fuelgauge.h"
#include "Key.h"
#include "adc.h"
#include "HardwareI2C.h"
#include "Led.h"
#include "Task.h"
#include "Begin.h"
#include "powerControl.h"
#include "Timer0.h"
#include "mcu_define.h"

extern float batteryLevelPercent;
extern float batteryTempC;
extern float batteryChargingPercentPerHour;
extern float batteryVoltage;

extern HardwareI2C i2c;
extern Charger mp2762;
extern Fuelgauge bq40z50;
