#include <Wire.h>

#include <ArduinoJson.h>

#define I2C_ADDR 0x34
#define ADC_BAT_ADDR 0
#define MOTOR_TYPE_ADDR 20                 // Magnetic encoder (44 pulses per revolution), gear ratio: 131 (default)
#define MOTOR_ENCODER_POLARITY_ADDR   21   // Range: 0 or 1, default is 0
#define MOTOR_FIXED_PWM_ADDR      31      // range -100~100, open loop ctrl
#define MOTOR_FIXED_SPEED_ADDR    51      // closed loop ctrl, pulses per 10ms, range typically Â±50
#define MOTOR_ENCODER_TOTAL_ADDR  60      // total pulse count, travel distance can be determined

// Distance-Traveled = (Pulse-Count/Pulses-per-Revolution) * (3.14159*Wheel-Diameter)

uint8_t MotorType = 3;                   // JGB style motor = 3
uint8_t MotorEncoderPolarity = 0;

int8_t MAX_SPD = 30;

int8_t car_forward[4] = { MAX_SPD, -MAX_SPD, -MAX_SPD,  MAX_SPD};  // Forward movement
int8_t car_back[4]    = {-MAX_SPD,  MAX_SPD,  MAX_SPD, -MAX_SPD};  // Backward movement
int8_t car_stop[4]    = {       0,        0,        0,        0};  // Full stop 

bool WireWriteDataArray(uint8_t reg, uint8_t *val, unsigned int len) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  for (unsigned int i = 0; i < len; i++) {
    Wire.write(val[i]);
  }
  return (Wire.endTransmission() == 0);
}

// handles signed input --> unsigned input casting
bool WireWriteDataArray(uint8_t reg, int8_t *val, unsigned int len) {
  return WireWriteDataArray(reg, (uint8_t*)val, len);
}

void Running(int8_t running_mode[4]) // input is an array of 4 integers
{
  Serial.println("Starting Running...");
  Serial.println(Wire.requestFrom(I2C_ADDR, 2));
  WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR,running_mode,4);  //Perform the corresponding action.
  delay(1000);
  WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR,car_stop,4);      // stop, works bc global var
  Serial.println(Wire.requestFrom(I2C_ADDR, 2));
  Serial.println("Ending Running...");
  delay(1000);

}

// Sample receive and then motor control using UART (115200 baud) and ArduinoJSON by Benoit Blanchon
// EX: data is a JSON structured command with a priority attachment function
// JSON format: command, priority, speed (keep format as condensed as possible bc of small SRAM)

// sample JSON (cmd = command, fwd = forward, pri = priority, spd = speed)
struct UART_DATA    // This variable is where received UART data would be but for now will be from CMD[]
{
  const char* command;
  int priority;
  int speed;
};

UART_DATA uartDATA;   // Initialization of struct as uartDATA

DynamicJsonDocument doc(200);   // dynamically allocate 200 bytes of memory in the heap
char CMD[] = "{\"cmd\":\"FWD\",\"pri\":1,\"spd\":30}";    // hard coded command for now but needs to be replaced with whatever data stream from UART line 

DeserializationError error = deserializeJson(doc, CMD);  // Deserialize the JSON packet and store into an error for checking

/*
if(error)               needs to be in a function or gives unqualified-id error
{
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}
*/ 

// Get values from CMD[]
const char* command = doc["cmd"];
int priority = doc["pri"];
int speed = doc["spd"];         // This takes the parsed data and places it into normal variables instead of char types

void dataRead()                 // point to parsed data and use for checks (right now speed is not used)
{
  uartDATA.command = command;   // once in use we can convert fwd, back, stop etc into enums instead of hard coding command checks
  uartDATA.priority = priority;
  uartDATA.speed = speed;       // now FWD is the command with priority 1 and speed of 30 

  if(strcmp(uartDATA.command, "FWD") == 0)    // once in use we can convert fwd, back, stop etc into enums instead of hard coding command checks
  {
    Running(car_forward);
  }
  else if(strcmp(uartDATA.command, "BACK") == 0)
  {
    Running(car_back);
  }
  else if(strcmp(uartDATA.command, "STOP") == 0)
  {
    Running(car_stop);
  }
}

void setup() {
  Wire.begin();
  delay(200);
  WireWriteDataArray(MOTOR_TYPE_ADDR,&MotorType,1);
  delay(5);
  WireWriteDataArray(MOTOR_ENCODER_POLARITY_ADDR,&MotorEncoderPolarity,1);
  delay(2000);

  // UART init for dataRead() example
  Serial.begin(115200);   // Init UART over onboard USB at 115200 baud rate
}

void loop() {
  //Running(car_forward);              
  Running(car_back);
  Running(car_stop);
  
}