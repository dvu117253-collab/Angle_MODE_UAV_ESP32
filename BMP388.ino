// #include <math.h>
// #define GRAVITY 9.8f
// #define dt 0.005
// typedef struct {
//     // State
//   float height = 0.0f;   // m
//   float vel_z  = 0.0f;   // m/s

//   // Covariance matrix P
//   float P00 = 1.0f;
//   float P01 = 0.0f;
//   float P10 = 0.0f;
//   float P11 = 1.0f;

//   // Noise tuning
//   float q_height = 0.01f;
//   float q_vel_z  = 0.30f;
//   float r_baro   = 0.80f;
// } AltitudeKalman2D;
// AltitudeKalman2D altKF;

// float pressure0_pa = 101325.0f;

// float calibratePressure0() {
//   const int N = 200;
//   float sum = 0.0f;

//   for (int i = 0; i < N; i++) {
//     float pressure = bmp.pressure();  // thay bằng hàm đọc BMP388 của bạn
//     sum += pressure;
//     delay(10);
//   }

//   return sum / N;
// }

// void updateAltitudeKalman2D(float height_baro, float accZ_ms2, float dt) {
//   if (dt <= 0.0f || dt > 0.1f) return;

//   altKF.height = altKF.height + altKF.vel_z * dt + 0.5f * accZ_ms2 * dt * dt;

//   altKF.vel_z = altKF.vel_z + accZ_ms2 * dt;

//   float P00 = altKF.P00 + dt * (altKF.P01 + altKF.P10) + dt * dt * altKF.P11 + altKF.q_height;

//   float P01 = altKF.P01 + dt * altKF.P11;
//   float P10 = altKF.P10 + dt * altKF.P11;
//   float P11 = altKF.P11 + altKF.q_vel_z;

//   altKF.P00 = P00;
//   altKF.P01 = P01;
//   altKF.P10 = P10;
//   altKF.P11 = P11;

//   float y = height_baro - altKF.height;
//   float S = altKF.P00 + altKF.r_baro;

//   float K0 = altKF.P00 / S;
//   float K1 = altKF.P10 / S;

//   altKF.height = altKF.height + K0 * y;
//   altKF.vel_z  = altKF.vel_z  + K1 * y;

//   float P00_old = altKF.P00;
//   float P01_old = altKF.P01;
//   float P10_old = altKF.P10;
//   float P11_old = altKF.P11;

//   altKF.P00 = (1.0f - K0) * P00_old;
//   altKF.P01 = (1.0f - K0) * P01_old;
//   altKF.P10 = P10_old - K1 * P00_old;
//   altKF.P11 = P11_old - K1 * P01_old;
// }
// void bmp388_setup() {
//   Wire.begin(I2C_SDA, I2C_SCL);
//   bmp.setPressureOversampling(BMP3_OVERSAMPLING_16X); //đo 16 lần rồi lấy TB
//   bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_15);// LPF max
//   bmp.setOutputDataRate(BMP3_ODR_50_HZ); //50Hz
// }
// float pressureToHeight(float pressure_pa, float pressure0_pa) {
//   return 44330.0f * (1.0f - powf(pressure_pa / pressure0_pa, 0.1903f));
// }
// void get_altitude() {

// }