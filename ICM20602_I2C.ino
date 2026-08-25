void icm_init() {
  Wire.beginTransmission(0x69);
  Wire.write(0x6B);     // PWR_MGMT_1
  Wire.write(0x01);     // PLL clock, wake up
  Wire.endTransmission();

  Wire.beginTransmission(0x69);
  Wire.write(0x1B);     // GYRO_CONFIG
  Wire.write(0x18);     // 2000dps
  Wire.endTransmission();

  Wire.beginTransmission(0x69);
  Wire.write(0x1C);     // ACCEL_CONFIG
  Wire.write(0x18);     // 16g
  Wire.endTransmission();

  Wire.beginTransmission(0x69);
  Wire.write(0x1A);     // DLPF_GYRO
  Wire.write(0x06);     //  5Hz
  Wire.endTransmission();

  Wire.beginTransmission(0x69);
  Wire.write(0x1D);     // DLPF_ACC
  Wire.write(0x06);     // 5Hz
  Wire.endTransmission();
  delay(100);
}
// void gyro_signals() {

//   Wire.beginTransmission(0x69);
//   Wire.write(0x3B);              // bắt đầu từ ACCEL_XOUT_H
//   Wire.endTransmission(false);   // giữ bus
//   Wire.requestFrom(0x69, 14);    // đọc 14 byte

//   int16_t AccXLSB = Wire.read() << 8 | Wire.read();
//   int16_t AccYLSB = Wire.read() << 8 | Wire.read();
//   int16_t AccZLSB = Wire.read() << 8 | Wire.read();

//   Wire.read(); Wire.read();    

//   int16_t GyroX = Wire.read() << 8 | Wire.read();
//   int16_t GyroY = Wire.read() << 8 | Wire.read();
//   int16_t GyroZ = Wire.read() << 8 | Wire.read();

//   AccX = (float)AccXLSB / 2048.0;     // ±16g
//   AccY = (float)AccYLSB / 2048.0;
//   AccZ = (float)AccZLSB / 2048.0;

//   AccX += offset_accX;
//   AccY += offset_accY;
//   AccZ += offset_accZ;

//   RatePitch  = (float)GyroX / 16.4;    // ±2000 dps
//   RateRoll = -(float)GyroY / 16.4;
//   RateYaw   = (float)GyroZ / 16.4;

//   AnglePitch  = atan2(AccY, sqrt(AccX * AccX + AccZ * AccZ)) * 180.0 / PI;
//   AngleRoll = atan2(AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0 / PI;
// }
void CalibRate() {

  for(RateCalibNumber = 0; RateCalibNumber < 3000; RateCalibNumber++) {
    gyro_signals();
    RateRollCalib += RateRoll;
    RatePitchCalib += RatePitch;
    RateYawCalib += RateYaw;
    delay(1);
  }
  RateRollCalib /= 3000;
  RatePitchCalib /= 3000;
  RateYawCalib /= 3000;
  if(RateCalibNumber > 2800) {
    Serial.println("-------------CALIBRATION DONE-----------------");
    Serial.print(RateRollCalib,2); Serial.print(", ");
    Serial.print(RatePitchCalib,2); Serial.print(", ");
    Serial.println(RateYawCalib,2); 
  }

}