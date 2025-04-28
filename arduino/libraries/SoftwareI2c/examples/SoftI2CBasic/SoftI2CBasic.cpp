#include <Arduino.h>
#include <SoftI2C.h>

// 定义I2C引脚
#define SCL_PIN 2  // 将此更改为您想使用的SCL引脚
#define SDA_PIN 3  // 将此更改为您想使用的SDA引脚

// 演示用的I2C设备地址
#define I2C_DEVICE_ADDR 0x68  // 常见地址，如MPU6050、DS3231等

// 创建SoftI2C对象
SoftI2C i2c(SCL_PIN, SDA_PIN);

void setup() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("软件I2C库基本示例");
  
  // 初始化I2C
  i2c.begin();
  
  // 检测设备是否存在
  Serial.print("检测设备 0x");
  Serial.print(I2C_DEVICE_ADDR, HEX);
  Serial.print("... ");
  
  if (i2c.deviceAvailable(I2C_DEVICE_ADDR)) {
    Serial.println("找到了！");
  } else {
    Serial.println("未找到！");
  }
}

void loop() {
  // 示例：从寄存器0x00读取一个字节
  Serial.print("读取寄存器 0x00: ");
  uint8_t value = i2c.read(I2C_DEVICE_ADDR, 0x00);
  Serial.print("0x");
  if (value < 0x10) Serial.print("0");
  Serial.println(value, HEX);
  
  // 示例：向寄存器0x01写入一个递增值
  static uint8_t counter = 0;
  counter++;
  
  Serial.print("写入寄存器 0x01: 0x");
  if (counter < 0x10) Serial.print("0");
  Serial.println(counter, HEX);
  
  bool success = i2c.write(I2C_DEVICE_ADDR, 0x01, counter);
  
  if (success) {
    Serial.println("写入成功！");
  } else {
    Serial.println("写入失败！");
  }
  
  // 示例：读取写入的值进行验证
  Serial.print("验证读取寄存器 0x01: ");
  value = i2c.read(I2C_DEVICE_ADDR, 0x01);
  Serial.print("0x");
  if (value < 0x10) Serial.print("0");
  Serial.println(value, HEX);
  
  if (value == counter) {
    Serial.println("验证成功！");
  } else {
    Serial.println("验证失败！");
  }
  
  Serial.println("------------------------");
  delay(1000);  // 等待1秒
} 