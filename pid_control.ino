// float LastDterm = 0;
// float lowPassFilter(float input, float previousOutput, float alpha) {
//   return alpha * previousOutput + (1 - alpha) * input;
// }
void pid_equation(float Error, float P, float I, float D, float PrevError, float PrevIterm, float I_lim, float PID_lim) {
  float Pterm = P * Error;
  float Iterm = PrevIterm + I*(Error+PrevError)*0.005/2;
  Iterm = constrain(Iterm, -I_lim, I_lim);
  float Dterm = D * (Error - PrevError) / 0.005;
  // float Dterm = lowPassFilter(rawDterm, LastDterm, 0.7);
  // LastDterm = Dterm;
  float PIDOutput = Pterm + Iterm + Dterm;
  PIDOutput = constrain(PIDOutput, -PID_lim, PID_lim);
  PIDReturn[0] = PIDOutput;
  PIDReturn[1] = Error;
  PIDReturn[2] = Iterm;
}
void PID_Angle(float Roll, float Pitch, float Roll_desir, float Pitch_desir) {
  ErrorAngleRoll = Roll_desir - Roll;
  pid_equation(ErrorAngleRoll, PAngleRoll, IAngleRoll, DAngleRoll, PrevErrorAngleRoll, PrevItermAngleRoll, 200, 400);
  DesiredRateRoll = PIDReturn[0];
  PrevErrorAngleRoll = PIDReturn[1];
  PrevItermAngleRoll = PIDReturn[2];
  ErrorAnglePitch = Pitch_desir - Pitch;
  pid_equation(ErrorAnglePitch, PAnglePitch, IAnglePitch, DAnglePitch, PrevErrorAnglePitch, PrevItermAnglePitch, 200, 400);
  DesiredRatePitch =PIDReturn[0];
  PrevErrorAnglePitch = PIDReturn[1];
  PrevItermAnglePitch = PIDReturn[2];
}
void PID_Rate(float RateRoll, float RatePitch, float RateYaw, float DesiredRateRoll, float DesiredRatePitch, float DesiredRateYaw) {
  ErrorRateRoll = DesiredRateRoll - RateRoll;
  pid_equation(ErrorRateRoll, PRateRoll, IRateRoll, DRateRoll, PrevErrorRateRoll, PrevItermRateRoll,200,400);
  InputRoll = PIDReturn[0];
  PrevErrorRateRoll = PIDReturn[1];
  PrevItermRateRoll = PIDReturn[2];

  ErrorRatePitch = DesiredRatePitch - RatePitch;
  pid_equation(ErrorRatePitch, PRatePitch, IRatePitch, DRatePitch, PrevErrorRatePitch, PrevItermRatePitch,200,400);
  InputPitch = PIDReturn[0];
  PrevErrorRatePitch = PIDReturn[1];
  PrevItermRatePitch = PIDReturn[2];

  ErrorRateYaw = DesiredRateYaw - RateYaw;
  pid_equation(ErrorRateYaw, PRateYaw, IRateYaw, DRateYaw, PrevErrorRateYaw, PrevItermRateYaw,500,500);
  InputYaw = PIDReturn[0];
  PrevErrorRateYaw = PIDReturn[1];
  PrevItermRateYaw = PIDReturn[2];
  // Serial.print("ERRORYAW = " + String(DesiredRateYaw) +"   ");
}
void reset_pid() {
  PrevErrorRateRoll = 0;
  PrevErrorRatePitch = 0;
  PrevErrorRateYaw = 0;
  PrevItermRateRoll = 0;
  PrevItermRatePitch = 0;
  PrevItermRateYaw = 0;

  PrevErrorAngleRoll = 0;
  PrevErrorAnglePitch = 0;
  PrevItermAngleRoll = 0;
  PrevItermAnglePitch = 0;
}