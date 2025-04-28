# SoftI2C Arduino库

这是一个基于Arduino平台的软件I2C库，无需使用硬件I2C外设，可以使用任意两个数字引脚实现I2C通信。

## 特性

- 支持任意数字引脚作为SCL和SDA
- 支持可调节的时钟频率
- 提供完整的I2C协议实现
- 支持多字节读写操作
- 支持设备检测功能

## 安装

将此文件夹复制到Arduino IDE的库文件夹中，或者放在项目的lib文件夹下。

## 使用方法

### 基本用法

```cpp
#include <SoftI2C.h>

// 定义I2C引脚（SCL, SDA）
#define SCL_PIN 2
#define SDA_PIN 3

// 创建SoftI2C对象，默认速度为100kHz
SoftI2C i2c(SCL_PIN, SDA_PIN);

void setup() {
  Serial.begin(115200);
  
  // 初始化I2C
  i2c.begin();
  
  // 检测I2C设备是否存在（0x68是一个常见的I2C地址）
  if (i2c.deviceAvailable(0x68)) {
    Serial.println("设备已找到！");
  } else {
    Serial.println("未找到设备！");
  }
}

void loop() {
  // 读取I2C设备寄存器示例
  uint8_t value = i2c.read(0x68, 0x00);
  Serial.print("寄存器值：");
  Serial.println(value);
  
  // 写入I2C设备寄存器示例
  i2c.write(0x68, 0x01, 0x42);
  
  delay(1000);
}
```

### 高级用法

#### 多字节读写

```cpp
// 写入多个字节
uint8_t dataToWrite[3] = {0x01, 0x02, 0x03};
i2c.write(0x68, 0x00, dataToWrite, 3);

// 读取多个字节
uint8_t dataBuffer[10];
if (i2c.read(0x68, 0x00, dataBuffer, 10)) {
  // 成功读取10个字节
  for (int i = 0; i < 10; i++) {
    Serial.print(dataBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
```

#### 使用不同的时钟频率

```cpp
// 创建一个400kHz的I2C总线
SoftI2C fastI2C(SCL_PIN, SDA_PIN, 400000);
fastI2C.begin();
```

## API参考

### 构造函数

- `SoftI2C(uint8_t sclPin, uint8_t sdaPin, uint32_t clockFreq = 100000)` - 创建SoftI2C对象

### 方法

- `void begin()` - 初始化I2C总线
- `bool write(uint8_t deviceAddr, uint8_t regAddr, uint8_t data)` - 写入单个字节
- `bool write(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint16_t length)` - 写入多个字节
- `uint8_t read(uint8_t deviceAddr, uint8_t regAddr)` - 读取单个字节
- `bool read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *buffer, uint16_t length)` - 读取多个字节
- `bool deviceAvailable(uint8_t deviceAddr)` - 检测设备是否存在

## 注意事项

1. 请确保I2C总线上有适当的上拉电阻（典型值为4.7kΩ）
2. 软件I2C比硬件I2C慢，不适合高速应用
3. 某些设备可能需要特定的时序要求，可能需要调整延迟时间
4. 长线缆可能导致通信问题，请保持连接线尽可能短 