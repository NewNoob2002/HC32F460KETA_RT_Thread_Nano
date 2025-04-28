#pragma once

#include <stdint.h>

#define BeginDebug 1

void Debugprint_Begin(const char *str, uint32_t time);

void I2cBegin();
void I2cEnd();
void Mp2762Begin();
void Bq40z50Begin();


