#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <Arduino.h>
#include <hc32_ddl.h>

// HC32F460芯片GPIO寄存器直接访问宏定义
// 端口定义
#define PORT_PORTA      ((en_port_t)0)
#define PORT_PORTB      ((en_port_t)1)
#define PORT_PORTC      ((en_port_t)2)
#define PORT_PORTD      ((en_port_t)3)
#define PORT_PORTE      ((en_port_t)4)
#define PORT_PORTH      ((en_port_t)5)

// 引脚掩码
#define PIN_MASK(pin)   (1ul << (pin))

// 直接设置/清除/读取引脚状态的宏（基于HC32F460 DDL库函数）
#define PIN_SET(port, pin)      PORT_SetBits((en_port_t)(port), PIN_MASK(pin))
#define PIN_RESET(port, pin)    PORT_ResetBits((en_port_t)(port), PIN_MASK(pin))
#define PIN_READ(port, pin)     (PORT_GetBit((en_port_t)(port), (en_pin_t)PIN_MASK(pin)) == Set)
#define PIN_TOGGLE(port, pin)   PORT_Toggle((en_port_t)(port), PIN_MASK(pin))

class SoftI2C {
private:
    uint8_t _sclPort;   // SCL端口号 (0=A, 1=B, 2=C...)
    uint8_t _sclPin;    // SCL引脚号
    uint8_t _sdaPort;   // SDA端口号
    uint8_t _sdaPin;    // SDA引脚号
    uint32_t _delay;    // 用于控制I2C速度的延迟（微秒）

    // GPIO快速操作内联函数
    inline void sdaHigh();   // SDA引脚设为高电平
    inline void sdaLow();    // SDA引脚设为低电平
    inline void sclHigh();   // SCL引脚设为高电平
    inline void sclLow();    // SCL引脚设为低电平
    inline bool sdaRead();   // 读取SDA引脚状态
    
    inline void delayMicroseconds(uint32_t us); // 延时函数
    
    // I2C协议操作
    void start();     // 开始信号
    void stop();      // 停止信号
    bool writeByte(uint8_t data);  // 写一个字节
    uint8_t readByte(bool ack);    // 读一个字节

public:
    // 构造函数, (端口0-5, A-H), SCL引脚, SDA引脚, 速度(Hz)
    // 默认速率400kHz (Fast Mode)
    SoftI2C(uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t clockFreq = 400000);
    
    // 初始化
    void begin();
    
    // 向指定设备的指定寄存器写入数据
    bool write(uint8_t deviceAddr, uint8_t regAddr, uint8_t data);
    
    // 向指定设备的指定寄存器写入多个字节
    bool write(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint16_t length);
    
    // 从指定设备的指定寄存器读取数据
    uint8_t read(uint8_t deviceAddr, uint8_t regAddr);
    
    // 从指定设备的指定寄存器读取多个字节
    bool read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *buffer, uint16_t length);
    
    // 检测设备是否存在
    bool deviceAvailable(uint8_t deviceAddr);
    
    // 设置时钟频率
    void setClock(uint32_t clockFreq);
};

#endif // SOFT_I2C_H 