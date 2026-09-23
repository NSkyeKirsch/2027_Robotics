#ifndef UART_Handler_H 
#define UART_Handler_H

#include <Arduino.h>

enum COMMAND : uint8_t 
{
  CMD_INVALID = 0,
  CMD_FWD,
  CMD_BACK,
  CMD_STOP
};

struct UART_DATA 
{
  COMMAND command;
  int priority;
  int speed;
};

// make uartDATA accessible from main
extern UART_DATA uartDATA;

// Parse JSON
void dataRead(char* jsonData);

// Read UART line
void readUART();

#endif