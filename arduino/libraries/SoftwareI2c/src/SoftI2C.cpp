#include "SoftI2C.h"

// 构造函数
SoftI2C::SoftI2C(uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t clockFreq) {
    _sclPort = sclPort;
    _sclPin = sclPin;
    _sdaPort = sdaPort;
    _sdaPin = sdaPin;
    
    setClock(clockFreq);
}

// 设置时钟频率
void SoftI2C::setClock(uint32_t clockFreq) {
    // 计算延迟时间（微秒）
    _delay = 1000000 / clockFreq / 2; // 半个周期的延迟
    
    // 对于I2C通信，确保最小延时
    if (clockFreq > 100000) {
        _delay = 4; // 固定延时4μs，对应约100kHz
    } else {
        _delay = 5; // 低速模式用更长延时
    }
}

// 初始化
void SoftI2C::begin() {
    // 配置SCL引脚为输出
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H; // 使用高驱动能力
    
    // 配置SCL引脚
    PORT_Init((en_port_t)_sclPort, PIN_MASK(_sclPin), &stcPortInit);
    
    // 配置SDA引脚
    PORT_Init((en_port_t)_sdaPort, PIN_MASK(_sdaPin), &stcPortInit);
    
    // 确保总线空闲状态（SCL和SDA都为高）
    PIN_SET(_sclPort, _sclPin);
    PIN_SET(_sdaPort, _sdaPin);
    
    // 初始化后添加额外延时以稳定总线
    delayMicroseconds(100);
}

// SDA设为高电平（释放总线，依靠上拉电阻）
inline void SoftI2C::sdaHigh() {
    // 切换为输入模式，让上拉电阻将线拉高
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Enable;
    PORT_Init((en_port_t)_sdaPort, PIN_MASK(_sdaPin), &stcPortInit);
    
    delayMicroseconds(_delay);
}

// SDA设为低电平
inline void SoftI2C::sdaLow() {
    // 切换为输出模式，拉低SDA线
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H; // 高驱动能力
    PORT_Init((en_port_t)_sdaPort, PIN_MASK(_sdaPin), &stcPortInit);
    
    PIN_RESET(_sdaPort, _sdaPin);
    delayMicroseconds(_delay);
}

// SCL设为高电平
inline void SoftI2C::sclHigh() {
    PIN_SET(_sclPort, _sclPin);
    
    // 简化时钟拉伸检测，增加超时时间
    uint8_t timeout = 20; // 简单循环计数超时
    
    while (!PIN_READ(_sclPort, _sclPin) && timeout > 0) {
        timeout--;
        delayMicroseconds(1);
    }
    
    delayMicroseconds(_delay);
}

// SCL设为低电平
inline void SoftI2C::sclLow() {
    PIN_RESET(_sclPort, _sclPin);
    delayMicroseconds(_delay);
}

// 读取SDA引脚状态
inline bool SoftI2C::sdaRead() {
    // 确保SDA是输入模式
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Enable;
    PORT_Init((en_port_t)_sdaPort, PIN_MASK(_sdaPin), &stcPortInit);
    
    // 添加短延时确保读取稳定
    delayMicroseconds(2);
    return PIN_READ(_sdaPort, _sdaPin);
}

// 优化的延时函数
inline void SoftI2C::delayMicroseconds(uint32_t us) {
    if (us == 0) return;
    delayMicroseconds(us);
}

// 开始信号
void SoftI2C::start() {
    // 确保SDA和SCL都为高
    sdaHigh();
    sclHigh();
    delayMicroseconds(5); // 延长起始条件前的稳定时间
    
    // SDA从高到低的跳变，当SCL为高时
    sdaLow();
    delayMicroseconds(5); // 延长起始条件的稳定时间
    
    // SCL拉低，准备数据传输
    sclLow();
}

// 停止信号
void SoftI2C::stop() {
    // 确保SCL为低
    sclLow();
    // SDA拉低
    sdaLow();
    delayMicroseconds(5); // 延长停止条件前的稳定时间
    
    // SCL拉高
    sclHigh();
    delayMicroseconds(5); // 延长停止条件的稳定时间
    
    // SDA从低到高的跳变，当SCL为高时
    sdaHigh();
    delayMicroseconds(10); // 额外等待以确保总线释放
}

// 写一个字节，返回ACK状态（true=ACK，false=NACK）
bool SoftI2C::writeByte(uint8_t data) {
    // 发送8位数据
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            sdaHigh();
        } else {
            sdaLow();
        }
        // 时钟SCL
        sclHigh();
        delayMicroseconds(2); // 确保高电平稳定
        sclLow();
        
        data <<= 1; // 为下一位准备
    }
    
    // 读取ACK
    sdaHigh(); // 释放SDA线
    delayMicroseconds(2); // 让从机有时间拉低SDA
    
    sclHigh();
    delayMicroseconds(5); // 延长ACK采样时间
    
    bool ack = !sdaRead(); // ACK是低电平
    sclLow();
    
    // 针对0x0B设备，如果收到NACK，再次尝试
    if (!ack && data == (0x0B << 1)) {
        delayMicroseconds(20); // 等待更长时间
        
        // 重新发送一遍
        sdaLow(); // 重新获取总线控制权
        delayMicroseconds(5);
        
        for (uint8_t i = 0; i < 8; i++) {
            if ((0x0B << 1) & (0x80 >> i)) {
                sdaHigh();
            } else {
                sdaLow();
            }
            sclHigh();
            delayMicroseconds(5); // 更长的稳定时间
            sclLow();
            delayMicroseconds(5);
        }
        
        // 再次读取ACK
        sdaHigh();
        delayMicroseconds(5);
        sclHigh();
        delayMicroseconds(5);
        ack = !sdaRead();
        sclLow();
    }
    
    return ack;
}

// 读一个字节，根据参数决定是否发送ACK
uint8_t SoftI2C::readByte(bool ack) {
    uint8_t data = 0;
    
    sdaHigh(); // 释放SDA线以便从设备写入
    
    // 读取8位数据
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1; // 为新数据移位
        
        sclHigh();
        delayMicroseconds(3); // 延长读取时间
        
        if (sdaRead()) {
            data |= 0x01;
        }
        sclLow();
        delayMicroseconds(2); // 短暂延时以稳定数据
    }
    
    // 发送ACK或NACK
    if (ack) {
        sdaLow(); // ACK（低电平）
    } else {
        sdaHigh(); // NACK（高电平）
    }
    
    sclHigh();
    delayMicroseconds(5); // 延长SCL高电平时间，确保从机接收到ACK/NACK
    sclLow();
    
    return data;
}

// 向指定设备的指定寄存器写入数据
bool SoftI2C::write(uint8_t deviceAddr, uint8_t regAddr, uint8_t data) {
    // 0x0B设备特殊处理
    bool is0B = (deviceAddr == 0x0B);
    uint8_t retries = is0B ? 3 : 1; // 为0x0B设备尝试多次
    
    while (retries > 0) {
        start();
        
        // 发送设备地址（写模式）
        if (!writeByte(deviceAddr << 1)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50); // 重试前稍作延时
                continue;
            }
            stop();
            return false;
        }
        
        // 0x0B设备特殊处理 - 在地址之后增加额外延时
        if (is0B) {
            delayMicroseconds(10);
        }
        
        // 发送寄存器地址
        if (!writeByte(regAddr)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);   
                continue;
            }
            stop();
            return false;
        }
        
        // 发送数据
        if (!writeByte(data)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);   
                continue;
            }
            stop();
            return false;
        }
        
        stop();
        return true;
    }
    
    return false;
}

// 向指定设备的指定寄存器写入多个字节
bool SoftI2C::write(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint16_t length) {
    // 0x0B设备特殊处理
    bool is0B = (deviceAddr == 0x0B);
    uint8_t retries = is0B ? 3 : 1; // 为0x0B设备尝试多次
    
    while (retries > 0) {
        start();
        
        // 发送设备地址（写模式）
        if (!writeByte(deviceAddr << 1)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return false;
        }
        
        // 0x0B设备特殊处理
        if (is0B) {
            delayMicroseconds(10);
        }
        
        // 发送寄存器地址
        if (!writeByte(regAddr)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return false;
        }
        
        // 发送多个数据字节
        for (uint16_t i = 0; i < length; i++) {
            if (!writeByte(data[i])) {
                if (--retries > 0) {
                    stop();
                    delayMicroseconds(50);
                    continue;
                }
                stop();
                return false;
            }
        }
        
        stop();
        return true;
    }
    
    return false;
}

// 从指定设备的指定寄存器读取数据
uint8_t SoftI2C::read(uint8_t deviceAddr, uint8_t regAddr) {
    uint8_t data = 0;
    
    // 0x0B设备特殊处理
    bool is0B = (deviceAddr == 0x0B);
    uint8_t retries = is0B ? 3 : 1; // 为0x0B设备尝试多次
    
    while (retries > 0) {
        start();
        
        // 发送设备地址（写模式）
        if (!writeByte(deviceAddr << 1)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return 0;
        }
        
        // 0x0B设备特殊处理
        if (is0B) {
            delayMicroseconds(10);
        }
        
        // 发送寄存器地址
        if (!writeByte(regAddr)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);   
                continue;
            }
            stop();
            return 0;
        }
        
        // 重新开始
        start();
        
        // 发送设备地址（读模式）
        if (!writeByte((deviceAddr << 1) | 0x01)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return 0;
        }
        
        // 读取数据（NACK表示读取结束）
        data = readByte(false);
        
        stop();
        return data;
    }
    
    return 0;
}

// 从指定设备的指定寄存器读取多个字节
bool SoftI2C::read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *buffer, uint16_t length) {
    if (length == 0) {
        return true;
    }
    
    // 0x0B设备特殊处理
    bool is0B = (deviceAddr == 0x0B);
    uint8_t retries = is0B ? 3 : 1; // 为0x0B设备尝试多次
    
    while (retries > 0) {
        start();
        
        // 发送设备地址（写模式）
        if (!writeByte(deviceAddr << 1)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return false;
        }
        
        // 0x0B设备特殊处理
        if (is0B) {
            delayMicroseconds(10);
        }
        
        // 发送寄存器地址
        if (!writeByte(regAddr)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return false;
        }
        
        // 重新开始
        start();
        
        // 发送设备地址（读模式）
        if (!writeByte((deviceAddr << 1) | 0x01)) {
            if (--retries > 0) {
                stop();
                delayMicroseconds(50);
                continue;
            }
            stop();
            return false;
        }
        
        // 读取多个数据字节，除了最后一个字节，都发送ACK
        for (uint16_t i = 0; i < length; i++) {
            buffer[i] = readByte(i < length - 1);
        }
        
        stop();
        return true;
    }
    
    return false;
}

// 检测设备是否存在
bool SoftI2C::deviceAvailable(uint8_t deviceAddr) {
    // 0x0B设备特殊处理
    bool is0B = (deviceAddr == 0x0B);
    uint8_t retries = is0B ? 3 : 1; // 为0x0B设备尝试多次
    
    while (retries > 0) {
        start();
        
        bool available = writeByte(deviceAddr << 1);
        
        stop();
        
        if (available) {
            return true;
        }
        
        if (--retries > 0) {
            delayMicroseconds(50);
        } else {
            return false;
        }
    }
    
    return false;
} 