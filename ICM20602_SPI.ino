#define offset_acc_x +0.004
#define offset_acc_y -0.010
#define offset_acc_z +0.017

#include <SPI.h>

#define CS_PIN 5
#define SPI_CLOCK 1000000  // 1 MHz

#define WHO_AM_I 0x75
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define CONFIG 0x1A
#define ACCEL_CONFIG2 0x1D
#define USER_CTRL 0x6A


SPISettings settings(SPI_CLOCK, MSBFIRST, SPI_MODE0);
// float roll,pitch;
// float AccX, AccY, AccZ;
// float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
// float AngleRoll, AnglePitch;

float KalmanAngleRoll = 0, KalmanUncertaintyAngleRoll = 2 * 2;
float KalmanAnglePitch = 0, KalmanUncertaintyAnglePitch = 2 * 2;
float Kalman1DOutput[] = {0, 0};

void setup_icm_20602() {
  // Serial.begin(115200);
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  ICM20602_Init();
 
  for (RateCalibrationNumber = 0; RateCalibrationNumber < 2000; RateCalibrationNumber++) {
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);
  }
  Serial.println("-----------------Calibration Done-------------------------");
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
  

}

void loop_icm_20602() {
  gyro_signals();
  
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;
  
  kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
  KalmanAngleRoll = Kalman1DOutput[0]; 
  KalmanUncertaintyAngleRoll = Kalman1DOutput[1];
  
  kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
  KalmanAnglePitch = Kalman1DOutput[0]; 
  KalmanUncertaintyAnglePitch = Kalman1DOutput[1];
    
  complementaryAngleRoll = 0.995 * (complementaryAngleRoll + RateRoll * 0.005) + 0.005 * AngleRoll;
  complementaryAnglePitch = 0.995 * (complementaryAnglePitch + RatePitch * 0.005) + 0.005 * AnglePitch;

  filterRoll= complementaryAngleRoll;
  filterPitch= complementaryAnglePitch;

  // Serial.print("R: ");
  // Serial.print(KalmanAngleRoll);
  // Serial.print(" P: ");
  // Serial.println(KalmanAnglePitch);
}

void ICM20602_Init() {
  // Thoát chế độ sleep
  writeRegister(PWR_MGMT_1, 0x00);
  delay(100);
  writeRegister(USER_CTRL, 0x10);  
  delay(10);
  // Cấu hình bộ lọc và thang đo (tương tự MPU6050 code)

  writeRegister(CONFIG, 0x05);    //10Hz Gyro
  writeRegister(ACCEL_CONFIG, 0x18);   // 16g
  writeRegister(GYRO_CONFIG, 0x18);      // 2000dps
  writeRegister(ACCEL_CONFIG2, 0x05);    //10Hz accel
  delay(100);
}

void gyro_signals(void) {
  // Đọc gia tốc
  int16_t AccXLSB, AccYLSB, AccZLSB;
  readAccel(AccXLSB, AccYLSB, AccZLSB);
  
  // Đọc gyro
  int16_t GyroX, GyroY, GyroZ;
  readGyro(GyroX, GyroY, GyroZ);
  
  // Chuyển đổi gyro (thang đo ±2000dps)
  RateRoll = (float)GyroX / 16.4f;
  RatePitch = (float)GyroY / 16.4f;
  RateYaw = (float)GyroZ / 16.4f;
  
  // Chuyển đổi gia tốc (thang đo ±16g)
  AccX = (float)AccXLSB / 2048.0f;
  AccY = (float)AccYLSB / 2048.0f;
  AccZ = (float)AccZLSB / 2048.0f;

  AccX = AccX + offset_acc_x;
  AccY = AccY + offset_acc_y;
  AccZ = AccZ + offset_acc_z;

  // Tính góc từ gia tốc kế
  AngleRoll = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 1 / (3.142 / 180);
  AnglePitch = -atan(AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 1 / (3.142 / 180);
}

void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement) {
  KalmanState = KalmanState + 0.005 * KalmanInput;
  KalmanUncertainty = KalmanUncertainty + 0.005 * 0.005 * 1 * 1;
  float KalmanGain = KalmanUncertainty * 1 / (1 * KalmanUncertainty + 3 * 3);
  KalmanState = KalmanState + KalmanGain * (KalmanMeasurement - KalmanState);
  KalmanUncertainty = (1 - KalmanGain) * KalmanUncertainty;
  Kalman1DOutput[0] = KalmanState; 
  Kalman1DOutput[1] = KalmanUncertainty;
}

void writeRegister(uint8_t reg, uint8_t value) {
  
  SPI.beginTransaction(settings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(reg & 0x7F); // Bit 7 là 0 cho ghi
  SPI.transfer(value);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
}

uint8_t readRegister(uint8_t reg) {
  
  SPI.beginTransaction(settings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(reg | 0x80); // Bit 7 là 1 cho đọc
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
  return value;
}

void readAccel(int16_t &x, int16_t &y, int16_t &z) {
  
  SPI.beginTransaction(settings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ACCEL_XOUT_H | 0x80);
  x = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  y = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  z = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
}

void readGyro(int16_t &x, int16_t &y, int16_t &z) {
  SPI.beginTransaction(settings);
  digitalWrite(CS_PIN, LOW);
  
  SPI.transfer(GYRO_XOUT_H | 0x80);
  x = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  y = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  z = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
}