#pragma once

#include <stdint.h>
// #include "settings.h"

#define TaskDebugFlag 0

void TaskDebug(const char *str, uint32_t time);
void checkBatteryLevels();
void BatteryCheckTask();

void handleChargerTask();
void WatchdogFeedTask();


void user_task_init();
