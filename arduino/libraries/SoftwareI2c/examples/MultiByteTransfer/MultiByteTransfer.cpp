#include <Arduino.h>
#include <SoftI2C.h>

// 定义I2C引脚
#define SCL_PIN 2  // 将此更改为您想使用的SCL引脚
#define SDA_PIN 3  // 将此更改为您想使用的SDA引脚

// 演示用的I2C设备地址
#define I2C_DEVICE_ADDR 0x68  // 常见地址，如MPU6050、DS3231等

// 创建SoftI2C对象，使用400kHz的频率
SoftI2C i2c(SCL_PIN, SDA_PIN, 400000);

void setup() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("软件I2C多字节传输示例");
  
  // 初始化I2C
  i2c.begin();
  
  // 检测设备是否存在
  if (i2c.deviceAvailable(I2C_DEVICE_ADDR)) {
    Serial.print("设备 0x");
    Serial.print(I2C_DEVICE_ADDR, HEX);
    Serial.println(" 已找到！");
  } else {
    Serial.print("设备 0x");
    Serial.print(I2C_DEVICE_ADDR, HEX);
    Serial.println(" 未找到！");
    Serial.println("请检查连接或更改设备地址后重试。");
    
    // 如果没有找到设备，就停止执行
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  // 示例1：写入多个字节到连续寄存器
  Serial.println("\n示例1：写入多个字节到寄存器 0x20-0x27");
  uint8_t dataToWrite[8];
  
  // 准备要写入的数据
  for (uint8_t i = 0; i < 8; i++) {
    dataToWrite[i] = i * 10; // 简单的数据模式：0, 10, 20, 30, 40, 50, 60, 70
  }
  
  // 显示要写入的数据
  Serial.print("写入数据: ");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(dataToWrite[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  // 写入多个字节
  bool writeSuccess = i2c.write(I2C_DEVICE_ADDR, 0x20, dataToWrite, 8);
  
  if (writeSuccess) {
    Serial.println("写入成功！");
  } else {
    Serial.println("写入失败！");
  }
  
  // 示例2：读取多个字节
  Serial.println("\n示例2：读取寄存器 0x20-0x27");
  uint8_t dataRead[8];
  
  // 读取多个字节
  bool readSuccess = i2c.read(I2C_DEVICE_ADDR, 0x20, dataRead, 8);
  
  if (readSuccess) {
    Serial.print("读取数据: ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(dataRead[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    // 验证读取的数据与写入的是否匹配
    bool dataMatch = true;
    for (uint8_t i = 0; i < 8; i++) {
      if (dataRead[i] != dataToWrite[i]) {
        dataMatch = false;
        break;
      }
    }
    
    if (dataMatch) {
      Serial.println("数据验证成功！");
    } else {
      Serial.println("数据验证失败！");
    }
  } else {
    Serial.println("读取失败！");
  }
  
  // 示例3：使用缓冲区进行大批量数据传输
  Serial.println("\n示例3：大批量数据传输（16字节）");
  
  // 创建一个更大的数据缓冲区
  uint8_t largeBuffer[16];
  
  // 填充缓冲区
  for (uint8_t i = 0; i < 16; i++) {
    largeBuffer[i] = 0xA0 + i; // 数据模式：0xA0, 0xA1, 0xA2, ...
  }
  
  // 写入大批量数据
  writeSuccess = i2c.write(I2C_DEVICE_ADDR, 0x30, largeBuffer, 16);
  
  if (writeSuccess) {
    Serial.println("大批量数据写入成功！");
  } else {
    Serial.println("大批量数据写入失败！");
  }
  
  // 读取大批量数据
  uint8_t readBuffer[16];
  readSuccess = i2c.read(I2C_DEVICE_ADDR, 0x30, readBuffer, 16);
  
  if (readSuccess) {
    Serial.println("大批量数据读取成功！");
    
    // 显示前几个字节
    Serial.print("数据样本: 0x");
    Serial.print(readBuffer[0], HEX);
    Serial.print(", 0x");
    Serial.print(readBuffer[1], HEX);
    Serial.print(", 0x");
    Serial.print(readBuffer[2], HEX);
    Serial.print(", ..., 0x");
    Serial.println(readBuffer[15], HEX);
  } else {
    Serial.println("大批量数据读取失败！");
  }
  
  Serial.println("\n等待5秒后重复测试...");
  delay(5000);
} 