#define arm_value 880
#define rest_value 800
void arm() {
  if(rcChannels[4] <= 2010 && rcChannels[4] >= 1700 ) {
  if(MotorFRInput<arm_value) MotorFRInput = arm_value;
  if(MotorFLInput<arm_value) MotorFLInput = arm_value;
  if(MotorBLInput<arm_value) MotorBLInput = arm_value;
  if(MotorBRInput<arm_value) MotorBRInput = arm_value;
  control_motor(MotorFRInput,MotorFLInput,MotorBRInput,MotorBLInput);
  } else {
    control_motor(rest_value,rest_value,rest_value,rest_value);
    reset_pid();
  }
}