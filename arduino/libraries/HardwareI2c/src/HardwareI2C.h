#ifndef HARDWARE_I2C_H
#define HARDWARE_I2C_H

#include <Arduino.h>
#include <hc32_ddl.h>

class HardwareI2C {
private:
    M4_I2C_TypeDef* i2cUnit;             // I2C外设单元
    uint32_t timeout;                    // 超时设置
    
    // 引脚配置
    uint8_t sclPort = 0;                // SCL引脚端口号
    uint8_t sclPin = 0;                 // SCL引脚编号
    uint8_t sdaPort = 0;                // SDA引脚端口号
    uint8_t sdaPin = 0;                 // SDA引脚编号

    // 初始化I2C外设
    en_result_t initialize(void);
    
    // 配置引脚
    void configPins();

public:
    // 构造函数 - 基本
    HardwareI2C(M4_I2C_TypeDef* unit, uint32_t timeoutValue = 0x40000ul);
    
    // 构造函数 - 带引脚配置
    HardwareI2C(M4_I2C_TypeDef* unit, 
               uint8_t sclPort, uint8_t sclPin, 
               uint8_t sdaPort, uint8_t sdaPin, 
               uint32_t timeoutValue = 0x40000ul);
    
    // 析构函数
    ~HardwareI2C();
    
    // 使用指定波特率初始化
    bool begin(uint32_t baudrate = 100000);

    bool end();
    
    // 主要方法
    bool writeRegister(uint8_t address, uint8_t reg, const uint8_t *data, uint32_t length, uint32_t u32TimeOut = 0);
    
    bool readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint32_t length, uint32_t u32TimeOut = 0);
    
    // 写入寄存器 16位数据
    bool writeRegister16(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut = 0);
    // 读取寄存器 16位数据
    bool readRegister16(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut = 0);
    
    bool writeRegister8(uint8_t address, uint8_t reg, uint8_t data, uint32_t u32TimeOut = 0);
    bool readRegister8(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut = 0);
    // 检查设备是否在线
    bool isDeviceReady(uint8_t addr, uint8_t trials = 3);
    
    // 设置超时时间（毫秒）
    void setTimeout(uint32_t timeoutMs);
    
    // 获取当前超时设置
    uint32_t getTimeout() const { return timeout; }
};

#endif // HARDWARE_I2C_H 