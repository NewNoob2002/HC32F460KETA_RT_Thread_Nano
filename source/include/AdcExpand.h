/**
 * HC32F460 ADC库
 * 用于简化ADC引脚和通道的初始化与读取操作
 */
#ifndef _ADC_LIB_H_
#define _ADC_LIB_H_

#include <Arduino.h>

bool adc_config_average(adc_device_t *adc_device, en_adc_avcnt_t avgCount);
bool adc_add_average_channel(adc_device_t *adc_device, uint32_t channel);





#endif