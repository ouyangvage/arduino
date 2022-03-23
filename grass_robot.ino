//ultrasionic pin configuration
//超声波引脚配置

//速度等级，-127~127
typedef struct 
{
  char line;
  char angular;
}Velocity;

//全局变量
Velocity current_velocity;      //机器主体速度
Velocity target_velocity;        //机器目标速度
unsigned char current_command;  //机器接收到的指令



//motor pin configuration
//电机引脚配置
const unsigned char motor_left_pin_1 = 2;
const unsigned char motor_left_pin_2 = 3;

const unsigned char motor_right_pin_1 = 4;
const unsigned char motor_right_pin_2 = 5;

//3 5 6 9 10 11 可直接用analogWrite 指定pwm
const unsigned char motor_left_enable = 9;
const unsigned char motor_right_enable = 10;

const unsigned char ultrasonic_echo = 11;
const unsigned char ultrasonic_trig = 12;

//distance measure
//距离测量
void ultrasonic_init()
{
  pinMode(ultrasonic_echo, INPUT);
  pinMode(ultrasonic_trig, OUTPUT);
}

float distance_measure(){
  digitalWrite(ultrasonic_trig, LOW);
  delayMicroseconds(20);
  digitalWrite(ultrasonic_trig, HIGH);
  delayMicroseconds(20);
  digitalWrite(ultrasonic_trig, LOW);
  return pulseIn(ultrasonic_echo, HIGH) * 340/2/10000;
}

//motor control
//电机控制
void motor_init()
{
  pinMode(motor_left_pin_1, OUTPUT);
  pinMode(motor_left_pin_2, OUTPUT);
  pinMode(motor_left_enable, OUTPUT);
  pinMode(motor_right_pin_1, OUTPUT);
  pinMode(motor_right_pin_2, OUTPUT);
  pinMode(motor_right_enable, OUTPUT);
}


//左电机正转
void motor_left(unsigned char pwm, boolean is_corotation)
{
  if(is_corotation == true)
  {
    digitalWrite(motor_left_pin_1, HIGH);
    digitalWrite(motor_left_pin_2, LOW);
  }
  else
  {
    digitalWrite(motor_left_pin_1, LOW);
    digitalWrite(motor_left_pin_2, HIGH);
  }
  analogWrite(motor_left_enable, pwm);
}

//左电机正转
void motor_right(unsigned char pwm, boolean is_corotation)
{
  if(is_corotation == true)
  {
    digitalWrite(motor_right_pin_1, HIGH);
    digitalWrite(motor_right_pin_2, LOW);
  }
  else
  {
    digitalWrite(motor_right_pin_1, LOW);
    digitalWrite(motor_right_pin_2, HIGH);
  }
  analogWrite(motor_right_enable, pwm);
}


//robot control
//机器控制
//机器的为两轮差速，转换为轮子的 pwm 小车的线速度 -127~ 127，角速度：-127~127 那么两个轮子转速将是-254 ~ 254
//速度等级转pwm 直流电机不是线性减速，所以暂时不考虑 pid 从速度到pwm 的转换 pwm 占空比值为 0~255 
void robot_control(Velocity velocity_robot)
{
  int velocity_wheel_left = velocity_robot.line + velocity_robot.angular;
  int velocity_wheel_right = velocity_robot.line - velocity_robot.angular;
  /*
  Serial.print("left pwm:");
  Serial.print(velocity_wheel_left);
  Serial.print("\t");
  Serial.print("left pwm:");
  Serial.println(velocity_wheel_right);
  */
  if(velocity_wheel_left >= 0)
  {
    motor_left(velocity_wheel_left, true);
  }
  else
  {
    motor_left((-1) * velocity_wheel_left, false);
  }

  if(velocity_wheel_right >= 0)
  {//速度等级转pwm 直流电机不是线性减速，所以暂时不考虑
    motor_right(velocity_wheel_right, true);
  }
  else
  {
    motor_right((-1) * velocity_wheel_right, false);
  }
}

//串口指令控制
void serial_read()
{
  //检查至少有一个可用字符
  if(Serial.available())
  {
    char char_read = Serial.read();
    //一个0-9 的ascii数字
    if(isDigit(char_read))
    {
       current_command = char_read; //更新当前指令
    }
  }
}

void robot_init()
{
  current_command = 0;
  
  current_velocity.line = 0;
  current_velocity.angular = 0;

  target_velocity.line = 0;
  target_velocity.angular = 0;
}

void robot_run()
{
  switch(current_command)
  {
    case '2': //后退
      target_velocity.line = -127;
      target_velocity.angular = 0;
      break;
      
     case '4': //左转
      target_velocity.line = 0;
      target_velocity.angular = 127;
      break;

     case '5': //停止
      target_velocity.line = 0;
      target_velocity.angular = 0;
      current_velocity.line = 0;
      current_velocity.angular = 0;
      break;
      
     case '6': //右转
      target_velocity.line = 0;
      target_velocity.angular = -127;
      break;

     case '8': //前进
        target_velocity.line = 127;
        target_velocity.angular = 0;
      break;
      
     default:
      break;
  }

  if(current_velocity.line > target_velocity.line)
  {
    current_velocity.line--;
  }
  else if(current_velocity.line < target_velocity.line)
  {
    current_velocity.line++;
  }

  if(current_velocity.angular > target_velocity.angular)
  {
    current_velocity.angular--;
  }
  else if(current_velocity.angular < target_velocity.angular)
  {
    current_velocity.angular++;
  }
  delay(20); //速度慢同步到目标速度
  robot_control(current_velocity);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  motor_init();
  ultrasonic_init();
  robot_init();
}

void loop() {
  serial_read();
  robot_run();
  /*
  Serial.print("current velocity.line: ");
  int current_velocity_line = current_velocity.line;
  Serial.print(current_velocity_line);
  Serial.print("\t");
  Serial.print("current velocity.angular: ");
  int current_velocity_angular = current_velocity.angular;
  Serial.println(current_velocity_angular);
  */
}
