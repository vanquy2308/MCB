#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>


#define SDA_PIN 21
#define SCL_PIN 22

BH1750 lightMeter(0x23); // Địa chỉ BH1750 (có thể là 0x5C)

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);  // Đảm bảo SDA và SCL được khai báo đúng
  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 đã kết nối thành công!");
  } else {
    Serial.println("Không tìm thấy BH1750, kiểm tra dây!");
  }
}

void loop() {
  float lux = lightMeter.readLightLevel();
  
  if (lux < 0) {
    Serial.println("[LỖI] Không đọc được dữ liệu từ BH1750!");
  } else {
    Serial.print("Cường độ ánh sáng: ");
    Serial.print(lux);
    Serial.println(" lx");
  }
  
  delay(1000);
}

