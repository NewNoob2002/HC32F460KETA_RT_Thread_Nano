#include <Arduino.h>
#include <SoftI2C.h>

// 定义I2C引脚
#define SCL_PIN 2  // 将此更改为您想使用的SCL引脚
#define SDA_PIN 3  // 将此更改为您想使用的SDA引脚

// 创建SoftI2C对象
SoftI2C i2c(SCL_PIN, SDA_PIN);

void setup() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("软件I2C总线扫描器");
  Serial.println("扫描地址范围: 0x03 - 0x77");
  
  // 初始化I2C
  i2c.begin();
  
  // 开始扫描
  scanI2CBus();
}

void loop() {
  // 每5秒重新扫描一次
  delay(5000);
  Serial.println("\n重新扫描...");
  scanI2CBus();
}

void scanI2CBus() {
  uint8_t deviceCount = 0;
  
  Serial.println("\n     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  
  for (uint8_t baseAddr = 0; baseAddr < 8; baseAddr++) {
    Serial.print(baseAddr * 16, HEX);
    Serial.print("0: ");
    
    for (uint8_t addr = 0; addr < 16; addr++) {
      uint8_t deviceAddr = baseAddr * 16 + addr;
      
      // 跳过保留地址
      if (deviceAddr < 0x03 || deviceAddr > 0x77) {
        Serial.print("   ");
        continue;
      }
      
      // 检测设备
      if (i2c.deviceAvailable(deviceAddr)) {
        Serial.print(deviceAddr < 16 ? " 0" : " ");
        Serial.print(deviceAddr, HEX);
        deviceCount++;
      } else {
        Serial.print(" --");
      }
    }
    
    Serial.println();
  }
  
  Serial.print("\n找到 ");
  Serial.print(deviceCount);
  Serial.println(" 个I2C设备");
} 