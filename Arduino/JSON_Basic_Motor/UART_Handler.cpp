#include "UART_Handler.h"
#include <ArduinoJson.h>
#include <string.h>

// Sample receive and then motor control using UART (115200 baud) and ArduinoJSON by Benoit Blanchon
// EX: data is a JSON structured command with a priority attachment function
// JSON format: command, priority, speed (keep format as condensed as possible bc of small SRAM)

// sample JSON (cmd = command, fwd = forward, pri = priority, spd = speed)   
// This variable is where received UART data would be but for now will be from CMD[]

UART_DATA uartDATA; // initialize the struct

DynamicJsonDocument doc(200); // Dynamically allocate 200 bytes of heap 
// Create buffer and index for UART data to be placed into and tracked
char uartBuffer[128];
uint8_t uartIndex = 0;

void readUART()
{
  while(Serial1.available() > 0)
  {
    char ReceivedByte = Serial1.read();

    // Check for \n (newline) indicates JSON message is complete
    if(ReceivedByte == '\n')
    {
      // Buffer == Index 
      uartBuffer[uartIndex] = '\0';

      // Parse JSON 
      dataRead(uartBuffer);

      // Reset for next command
      uartIndex = 0;
    }
    else 
    {
      // Add byte to buffer
      if(uartIndex < sizeof(uartBuffer) - 1)
      {
        uartBuffer[uartIndex] = ReceivedByte;
        uartIndex++;
      }
      // Buffer overflow
      else
      {
        uartIndex = 0;
      }
    }
  }
}

COMMAND parseCommand(const char* command)
{
  if(strcmp(command, "FWD") == 0)    // once in use we can convert fwd, back, stop etc into enums instead of hard coding command checks
    {
      return CMD_FWD;
    }
  else if(strcmp(command, "BACK") == 0)
    {
      return CMD_BACK;
    }
  else if(strcmp(command, "STOP") == 0)
    {
      return CMD_STOP;
    }
  return CMD_INVALID;
}

void dataRead(char* jsonData)                 // point to parsed data and use for checks (right now speed is not used)
{

  DeserializationError error = deserializeJson(doc, jsonData);  // Deserialize the JSON packet and store into an error for checking
  
  if(error)             
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  const char* command = doc["cmd"];

  uartDATA.command = parseCommand(command);   // once in use we can convert fwd, back, stop etc into enums instead of hard coding command checks
  uartDATA.priority = doc["pri"];
  uartDATA.speed = doc["spd"];       // now FWD is the command with priority 1 and speed of 30 

}