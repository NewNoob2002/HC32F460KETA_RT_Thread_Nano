#include "HardwareI2C.h"
#include <Arduino.h>

// 获取引脚功能复用类型
static en_port_func_t getI2CPinFunc(M4_I2C_TypeDef *i2c, bool isSCL)
{
    if (i2c == M4_I2C1)
    {
        return isSCL ? Func_I2c1_Scl : Func_I2c1_Sda;
    }
    else if (i2c == M4_I2C2)
    {
        return isSCL ? Func_I2c2_Scl : Func_I2c2_Sda;
    }
    else if (i2c == M4_I2C3)
    {
        return isSCL ? Func_I2c3_Scl : Func_I2c3_Sda;
    }
    return Func_Gpio;
}

// 获取端口指针
static en_port_t getPortInstance(uint8_t port)
{
    switch (port)
    {
    case 0:
        return PortA;
    case 1:
        return PortB;
    case 2:
        return PortC;
    case 3:
        return PortD;
    case 4:
        return PortE;
    case 5:
        return PortH;
    default:
        return PortA;
    }
}

// 获取引脚编号
static en_pin_t getPinIndex(uint8_t pin)
{
    switch (pin)
    {
    case 0:
        return Pin00;
    case 1:
        return Pin01;
    case 2:
        return Pin02;
    case 3:
        return Pin03;
    case 4:
        return Pin04;
    case 5:
        return Pin05;
    case 6:
        return Pin06;
    case 7:
        return Pin07;
    case 8:
        return Pin08;
    case 9:
        return Pin09;
    case 10:
        return Pin10;
    case 11:
        return Pin11;
    case 12:
        return Pin12;
    case 13:
        return Pin13;
    case 14:
        return Pin14;
    case 15:
        return Pin15;
    default:
        return Pin00;
    }
}

/**
 * @brief 构造函数，初始化I2C外设
 * @param unit I2C外设单元
 * @param timeoutValue 超时值
 */
HardwareI2C::HardwareI2C(M4_I2C_TypeDef* unit, uint32_t timeoutValue)
{
    i2cUnit = unit;
    timeout = timeoutValue;
    
    // 初始化I2C外设
    initialize();
}

/**
 * @brief 使用引脚号的构造函数
 * @param unit I2C外设单元
 * @param sclPort SCL引脚端口号
 * @param sclPin SCL引脚编号
 * @param sdaPort SDA引脚端口号
 * @param sdaPin SDA引脚编号
 * @param timeoutValue 超时值
 */
HardwareI2C::HardwareI2C(M4_I2C_TypeDef* unit, 
                         uint8_t sclPort, uint8_t sclPin, 
                         uint8_t sdaPort, uint8_t sdaPin, 
                         uint32_t timeoutValue)
{
    i2cUnit = unit;
    timeout = timeoutValue;
    
    // 保存引脚信息
    this->sclPort = sclPort;
    this->sclPin = sclPin;
    this->sdaPort = sdaPort;
    this->sdaPin = sdaPin;
    
    // 配置引脚
    configPins();
    
    // 使能I2C时钟
    if (unit == M4_I2C1) {
        PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_I2C1, Enable);
    } else if (unit == M4_I2C2) {
        PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_I2C2, Enable);
    } else if (unit == M4_I2C3) {
        PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_I2C3, Enable);
    }
    
    // // 初始化I2C外设
    // if (initialize() != Ok) {
    //     Serial.println("I2C初始化失败");
    // }
}

/**
 * @brief 配置引脚为I2C功能
 */
void HardwareI2C::configPins()
{
    en_port_t sclPortEnum = getPortInstance(sclPort);
    en_port_t sdaPortEnum = getPortInstance(sdaPort);
    en_pin_t sclPinEnum = getPinIndex(sclPin);
    en_pin_t sdaPinEnum = getPinIndex(sdaPin);
    
    // 配置引脚为I2C功能
    PORT_SetFunc(sclPortEnum, sclPinEnum, getI2CPinFunc(i2cUnit, true), Disable);
    PORT_SetFunc(sdaPortEnum, sdaPinEnum, getI2CPinFunc(i2cUnit, false), Disable);
}

/**
 * @brief 析构函数
 */
HardwareI2C::~HardwareI2C()
{
    // 禁用I2C外设
    I2C_Cmd(i2cUnit, Disable);
}

/**
 * @brief 初始化I2C外设
 * @return en_result_t 初始化结果
 */
en_result_t HardwareI2C::initialize(void)
{
    stc_i2c_init_t stcI2cInit;
    en_result_t enRet;
    float32_t fErr;

    // 复位I2C外设
    I2C_DeInit(i2cUnit);

    // 配置I2C参数
    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV8;
    stcI2cInit.u32Baudrate = 100000ul;  // 100KHz
    stcI2cInit.u32SclTime = 0ul;
    
    // 初始化I2C
    enRet = I2C_Init(i2cUnit, &stcI2cInit, &fErr);

    // 启用总线等待功能
    I2C_BusWaitCmd(i2cUnit, Enable);

    return enRet;
}

/**
 * @brief 初始化I2C外设并设置波特率
 * @param baudrate 波特率
 * @return bool 初始化结果
 */
bool HardwareI2C::begin(uint32_t baudrate)
{
    stc_i2c_init_t stcI2cInit;
    float32_t fErr = 0.0f;
    
    // 复位I2C外设
    I2C_DeInit(i2cUnit);

    // 配置I2C参数
    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV8;
    stcI2cInit.u32Baudrate = baudrate;
    stcI2cInit.u32SclTime = 0ul;
    
    // 初始化I2C
    if (Ok != I2C_Init(i2cUnit, &stcI2cInit, &fErr)) {
        return false;
    }
    // 启用总线等待功能
    I2C_BusWaitCmd(i2cUnit, Enable);
    
    return true;
}

bool HardwareI2C::end()
{
    I2C_Cmd(i2cUnit, Disable);
    return true;
}

/**
 * @brief 写入寄存器(1字节地址，1字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 要写入的数据
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::writeRegister8(uint8_t address, uint8_t reg, uint8_t data, uint32_t u32TimeOut)
{
    return writeRegister(address, reg, &data, 1, u32TimeOut);
}

/**
 * @brief 读取寄存器(1字节地址，1字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 数据存储地址
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::readRegister8(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut)
{
    return readRegister(address, reg, data, 1, u32TimeOut);
}

/**
 * @brief 写入寄存器(2字节地址，1字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 要写入的数据
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::writeRegister16(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut)
{
    return writeRegister(address, reg, data, 2, u32TimeOut);
}

/**
 * @brief 读取寄存器(2字节地址，1字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 数据存储地址
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::readRegister16(uint8_t address, uint8_t reg, uint8_t *data, uint32_t u32TimeOut)
{
    return readRegister(address, reg, data, 2, u32TimeOut);
}

/**
 * @brief 写入寄存器(1字节地址，多字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 数据缓冲区
 * @param length 数据长度
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::writeRegister(uint8_t address, uint8_t reg, const uint8_t *data, uint32_t length, uint32_t u32TimeOut)
{
    uint32_t actualTimeout = (u32TimeOut > 0) ? u32TimeOut : timeout;
    
    I2C_Cmd(i2cUnit, Enable);
    I2C_SoftwareResetCmd(i2cUnit, Enable);
    I2C_SoftwareResetCmd(i2cUnit, Disable);

    // 发送开始信号
    if (Ok == I2C_Start(i2cUnit, actualTimeout))
    {
        // 发送设备地址+写入位
        if (Ok == I2C_TransAddr(i2cUnit, address, I2CDirTrans, actualTimeout))
        {
            // 发送寄存器地址
            if (Ok == I2C_TransData(i2cUnit, &reg, 1, actualTimeout))
            {
                // 发送数据
                if (Ok == I2C_TransData(i2cUnit, (uint8_t *)data, length, actualTimeout))
                {
                    I2C_Stop(i2cUnit, actualTimeout);
                    I2C_Cmd(i2cUnit, Disable);
                    return true;
                }
            }
        }
    }

    I2C_Stop(i2cUnit, actualTimeout);
    I2C_Cmd(i2cUnit, Disable);
    return false;
}

/**
 * @brief 读取寄存器数据(1字节地址，多字节数据)
 * @param address 设备地址
 * @param reg 寄存器地址
 * @param data 数据缓冲区
 * @param length 数据长度
 * @param u32TimeOut 超时时间，0表示使用默认值
 * @return bool 操作结果
 */
bool HardwareI2C::readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint32_t length, uint32_t u32TimeOut)
{
    bool success = false;
    uint32_t actualTimeout = (u32TimeOut > 0) ? u32TimeOut : timeout;

    I2C_Cmd(i2cUnit, Enable);
    I2C_SoftwareResetCmd(i2cUnit, Enable);
    I2C_SoftwareResetCmd(i2cUnit, Disable);

    // 发送开始信号
    if (Ok == I2C_Start(i2cUnit, actualTimeout))
    {
        // 发送设备地址+写入位
        if (Ok == I2C_TransAddr(i2cUnit, address, I2CDirTrans, actualTimeout))
        {
            // 发送寄存器地址
            if (Ok == I2C_TransData(i2cUnit, &reg, 1, actualTimeout))
            {
                // 发送重复开始信号
                if (Ok == I2C_Restart(i2cUnit, actualTimeout))
                {
                    if (1 == length)
                    {
                        I2C_AckConfig(i2cUnit, I2c_NACK);
                    }

                    // 发送设备地址+读取位
                    if (Ok == I2C_TransAddr(i2cUnit, address, I2CDirReceive, actualTimeout))
                    {
                        // 使用带停止位的数据接收函数
                        if (Ok == I2C_MasterDataReceiveAndStop(i2cUnit, data, length, actualTimeout))
                        {
                            success = true;
                        }
                    }
                    // 恢复默认响应为ACK
                    I2C_AckConfig(i2cUnit, I2c_ACK);
                }
            }
        }
    }

    if (success != true)
    {
        I2C_Stop(i2cUnit, actualTimeout);
    }

    I2C_Cmd(i2cUnit, Disable);
    return success;
}

/**
 * @brief 设置超时时间（毫秒）
 * @param timeoutMs 超时时间（毫秒）
 */
void HardwareI2C::setTimeout(uint32_t timeoutMs)
{
    timeout = timeoutMs * SystemCoreClock / 1000;
}

/**
 * @brief 检查设备是否在线
 * @param addr 设备地址
 * @param trials 尝试次数
 * @return bool 设备是否在线
 */
bool HardwareI2C::isDeviceReady(uint8_t addr, uint8_t trials)
{
    bool result = false;
    uint8_t retryCount = 0;
    
    while (retryCount < trials && !result) {
        // 启用I2C外设
        I2C_Cmd(i2cUnit, Enable);
        
        // 软件复位
        I2C_SoftwareResetCmd(i2cUnit, Enable);
        I2C_SoftwareResetCmd(i2cUnit, Disable);
        
        // 发送起始信号
        if (Ok == I2C_Start(i2cUnit, timeout)) {
            // 发送设备地址(写操作)
            if (Ok == I2C_TransAddr(i2cUnit, addr, I2CDirTrans, timeout)) {
                result = true;
            }
        }
        
        // 发送停止信号
        I2C_Stop(i2cUnit, timeout);
        
        // 禁用I2C外设
        I2C_Cmd(i2cUnit, Disable);
        
        retryCount++;
        
        // 如果连接失败，延时一段时间再重试
        if (!result && retryCount < trials) {
            delay(5);
        }
    }
    
    return result;
}