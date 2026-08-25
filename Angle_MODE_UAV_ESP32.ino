// ch11[2] = throttle, ch11[0] = roll, ch11[1] = pitch, ch11[3] = yaw, 
//ch11[4] = SA, ch11[7] = SD, ch11[9] = S1, ch11[5] = SB, ch11[6] = SC, ch11[8] = SE
#include <Wire.h>
#include <Wire.h>
#include <Adafruit_BMP3XX.h>
// #define offset_accX -0.0
// #define offset_accY 0.0
// #define offset_accZ 0.0
#define ESC_FR 27
#define ESC_FL 25
#define ESC_BR 33
#define ESC_BL 26
#define BUZZER 14
#define LED 13
#define euler_max 20
#define rateyaw_max 90
#define LPF_Roll_Pitch_RX 0.1
#define LPF_RateYaw_RX 0.3
#define LPF_Throttle_RX 0.3

#define I2C_SDA 21
#define I2C_SCL 22
#define BMP399_ADDR 0x76
Adafruit_BMP3XX bmp;

unsigned long lastAltTime=0;

const int esc_1_channel=0, esc_2_channel=1, esc_3_channel=2, esc_4_channel=3;
const int freq = 391;  ////tương ứng 800 1600 ->1000 2000 
const int resolution = 11;

float AngleRoll, AnglePitch;
float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
float RateRollCalib = 0, RatePitchCalib = 0, RateYawCalib = 0;
int RateCalibNumber;
int MotorFRInput, MotorFLInput, MotorBRInput, MotorBLInput;
float DesiredAngleRoll, DesiredAnglePitch;
float ErrorAngleRoll, ErrorAnglePitch;

float DesiredRateRoll, DesiredRatePitch, DesiredRateYaw;
float ErrorRateRoll, ErrorRatePitch, ErrorRateYaw;
float InputRoll, InputThrottle, InputPitch, InputYaw;
float PrevErrorRateRoll, PrevErrorRatePitch, PrevErrorRateYaw;
float PrevItermRateRoll, PrevItermRatePitch, PrevItermRateYaw;
float PIDReturn[] = { 0, 0, 0 };
// float PReturn = 0.55;
float PRateRoll = 0.8;
float PRatePitch = PRateRoll;
float PRateYaw = 1.5;
float IRateRoll = 0;
float IRatePitch = IRateRoll;
float IRateYaw = 10;
float DRateRoll = 0.04;
float DRatePitch = DRateRoll;
float DRateYaw = 0;

float PrevErrorAngleRoll, PrevErrorAnglePitch;
float PrevItermAngleRoll, PrevItermAnglePitch;

float PAngleRoll = 10;
float PAnglePitch = 10;
float IAngleRoll = 0;
float IAnglePitch = 0;
float DAngleRoll = 0;
float DAnglePitch = 0;
// static const int CRSF_RX_PIN = 17;      
// static const int CRSF_TX_PIN = 4;          
// static const uint32_t CRSF_BAUD = 420000;   

// static uint16_t ch11[16];          
// static uint32_t lastRcPacketMs = 0;
float complementaryAngleRoll = 0.0f;
float complementaryAnglePitch = 0.0f;
float filterRoll = 0.0f;
float filterPitch = 0.0f;
uint32_t LoopTimer;
int rcChannels[16];
void icm_init();
void gyro_signals();
void CalibRate();

void setup_motor(){
  // configure LED PWM functionalitites
  ledcSetup(esc_1_channel, freq, resolution);  ledcAttachPin(ESC_FR, esc_1_channel);
  ledcSetup(esc_2_channel, freq, resolution);  ledcAttachPin(ESC_FL, esc_2_channel);
  ledcSetup(esc_3_channel, freq, resolution);  ledcAttachPin(ESC_BR, esc_3_channel);
  ledcSetup(esc_4_channel, freq, resolution);  ledcAttachPin(ESC_BL, esc_4_channel);

  control_motor(800,800,800,800); 
  delay(1000);
  control_motor(800,800,800,800); 
  delay(1000);
}
void control_motor(int m1, int m2, int m3, int m4){
    // Serial.print("FR= ");
    // Serial.print(m1);Serial.print("   ");
    // Serial.print("FL= ");
    // Serial.print(m2);Serial.print("   ");
    // Serial.print("BR= ");
    // Serial.print(m3);Serial.print("   ");
    // Serial.print("BL= ");
    // Serial.println(m4);
    ledcWrite(esc_1_channel, m1);
    ledcWrite(esc_2_channel, m2);
    ledcWrite(esc_3_channel, m3);
    ledcWrite(esc_4_channel, m4);

}
void Done_Music() {
  digitalWrite(BUZZER,HIGH); delay(500); digitalWrite(BUZZER,LOW); delay(100); 
  digitalWrite(BUZZER,HIGH); delay(500); digitalWrite(BUZZER,LOW); delay(100);
  digitalWrite(BUZZER,HIGH); delay(500); digitalWrite(BUZZER,LOW); delay(100); 
  digitalWrite(BUZZER,HIGH);
  delay(2000);
  digitalWrite(BUZZER,LOW);
}

void setup() {

  pinMode(BUZZER,OUTPUT);
  // pinMode(LED,OUTPUT);
  // digitalWrite(LED,HIGH);
  digitalWrite(BUZZER,LOW);
  setup_motor();
  delay(6000);

  Serial.begin(115200);
  delay(200);
  sbus_setup();
  // Serial2.begin(CRSF_BAUD, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
  // for (int i = 0; i < 16; i++) ch11[i] = 992;
  Serial.println("Please put the UAV static");
  setup_icm_20602();
  // bmp388_setup();
  // Wire.begin();               // ESP32 có thể dùng Wire.begin(SDA, SCL);
  // Wire.setClock(400000);      // 400kHz nếu board/sensor ổn định
  // icm_init();
  // CalibRate();
  // digitalWrite(LED,LOW);
  LoopTimer = micros();
  Done_Music();
}

void loop() {
  
  loop_icm_20602();
  //----- ICM20602_I2C ------
  // gyro_signals();
  // RateRoll -= RateRollCalib;
  // RatePitch -= RatePitchCalib;
  // RateYaw -= RateYawCalib;
  // -------------------------------

    read_sbus();
    // read_TX();
    DesiredAngleRoll = DesiredAngleRoll * (1 - LPF_Roll_Pitch_RX) + (((float)rcChannels[0] - 1500) / (500.0 / euler_max)) * LPF_Roll_Pitch_RX;
    DesiredAnglePitch = DesiredAnglePitch * (1 - LPF_Roll_Pitch_RX) + (((float)rcChannels[1] - 1500) / (500.0 / euler_max)) * LPF_Roll_Pitch_RX;
    InputThrottle = InputThrottle * (1 - LPF_Throttle_RX) + map(rcChannels[2],1000,2000,800,1600)*LPF_Throttle_RX;
    DesiredRateYaw = DesiredRateYaw * (1 - LPF_RateYaw_RX) + (((float)rcChannels[3] - 1500) / (500.0 / rateyaw_max)) * LPF_RateYaw_RX;
  // Serial.println(rcChannels[3]);
  // Serial.print("Desired_Roll: ");
  // Serial.print(DesiredAngleRoll);Serial.print(", ");
  // Serial.print("Desired_Pitch: ");
  // Serial.print(DesiredAnglePitch);Serial.print(", ");
  // Serial.print("Throttle: ");
  // Serial.print(InputThrottle);Serial.print(", ");
  // Serial.print("Desired_RateYaw: ");
  // Serial.println(DesiredRateYaw);
  PID_Angle(complementaryAngleRoll,complementaryAnglePitch,DesiredAngleRoll,DesiredAnglePitch);
  PID_Rate(RateRoll,RatePitch,RateYaw,DesiredRateRoll,DesiredRatePitch,DesiredRateYaw);
  // if(AccZ > 7) control_motor(800,800,800,800);

  MotorFRInput = (InputThrottle - InputRoll - InputPitch - InputYaw);
  MotorFLInput = (InputThrottle + InputRoll - InputPitch + InputYaw);
  MotorBRInput = (InputThrottle - InputRoll + InputPitch + InputYaw);
  MotorBLInput = (InputThrottle + InputRoll + InputPitch - InputYaw);
  MotorFRInput = constrain(MotorFRInput, 800, 1600);
  MotorFLInput = constrain(MotorFLInput, 800, 1600);
  MotorBRInput = constrain(MotorBRInput, 800, 1600);
  MotorBLInput = constrain(MotorBLInput, 800, 1600);
  arm();

  // Serial.print("Acc: ");
  // Serial.print(AccX,3);Serial.print(", ");
  // Serial.print(AccY,3);Serial.print(", ");
  // Serial.println(AccZ,3); Serial.print(", ");

  //   Serial.print(RateRoll);
  // Serial.print(" ");
  // Serial.print(RatePitch);
  // Serial.print(" ");
  // Serial.print(RateYaw);
  // Serial.print(" ");
  // Serial.print(-1);   // giá trị MIN cố định
  // Serial.print(" ");
  // Serial.println(1);  // giá trị MAX cố định

  // Serial.print("| Gyro= ");
  // Serial.print(RateRoll,2);Serial.print(", ");
  // Serial.print(RatePitch,2);Serial.print(", ");
  // Serial.println(RateYaw,2);
  // Serial.print("| Angle: ");
  // Serial.print(AngleRoll,2);Serial.print(", ");
  // Serial.println(AnglePitch,2);

  // Serial.print("AnglePitchFilter= ");
  // Serial.print(complementaryAnglePitch);Serial.print("   ");
  // Serial.print("AngleRollFilter= ");
  // Serial.print(complementaryAngleRoll);Serial.print("   ");
  // Serial.print("FR= ");
  // Serial.print(MotorFRInput);Serial.print("   ");
  //   Serial.print("FL= ");
  // Serial.print(MotorFLInput);Serial.print("   ");
  // Serial.print("BR= ");
  // Serial.print(MotorBRInput);Serial.print("   ");
  //   Serial.print("BL= ");
  // Serial.println(MotorBLInput);


  while (micros() - LoopTimer < (0.005 * 1000000));
  {
    LoopTimer = micros();
  }
}
