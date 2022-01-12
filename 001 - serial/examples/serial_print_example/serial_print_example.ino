/*
 * Simplified printing to the serial output.
 * 
 * Run this example with the Serial Monitor app open on your
 * computer to see the formatted data.
 * 
 * This example uses a baud rate of 115,200 but you can change that in the
 * setup() function.
 * 
 * In Arduino -> Preferences.. set Compiler Warnings to: More
 * 
 */

// When this macro is defined, the dbg macro will send messages out.
// If you comment out this macro definition (or just delete it) then
// the dbg message are omitted and the strings used to format the messages
// are omitted from the code build. You musty define this BEFORE you
// include serial_utils.h
#define DEBUG

#include "serial_utils.h"


 
void setup() 
{
  // Start the serial support
  Serial.begin(115200);

  // Scroll down a bit and say hello
  sout("\n\n\n\n\nHello"); 
  sout("Welcome to simplified debugging :)");
  dbg("The DEBUG macro is defined");

}

// A few global variables so we can show how to print them out.
uint8_t g_byte_val = 29;
int g_int_val = 97;
long g_long_val = 65537L;
float g_float_val = 123.456;

void loop() 
{
  // Simulate something going on that we want to show on the monitor
  long time_ms = millis();

  // measure how long this takes
  long start = micros();

  // Print out all the values.
  // To print the float we need to format it as a string first so the conversion
  // string we use to print it out is %s, not %f 
  sout("Time: %6ld ms, byte: 0x%2.2X, int: %5d, long: %11ld, float: %s",
      time_ms, g_byte_val, g_int_val, g_long_val, f2s(g_float_val, 2));

  // Make some changes so we aren't printing out the same values all the time
  g_byte_val *= 27;
  g_int_val += 375;
  g_long_val += time_ms; 
  g_float_val *= 1.002;

  long elapsed = micros() - start;

  // Just as an example we only print the elapsed time in the debug build
  // To turn this off, comment out the #define DEBUG statement at the top of the program
  dbg("Elapsed time: %ld us", elapsed);
  
  // wait a bit before we go around again
  delay(1000); // 1 second
}
