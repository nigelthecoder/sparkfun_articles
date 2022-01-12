/** \file serial_utils.cpp
 *  \brief Functions to support printf-like serial output.
 *
 */

#include "serial_utils.h"

void serial_printf(const char* fmt, ...)
{
  // buffer to assemble the text into.
  // NOTE: limited size!
  char buf[128]; 

  // format the output string
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, 127, fmt, args);

  // send it out
  Serial.println(buf);

  // tidy up
  va_end (args);
}

// A global string used to format the floats
String _f2s_buffer;

const char* f2s(float& value, uint8_t places)
{
  _f2s_buffer = String(value, places);
  return _f2s_buffer.c_str();
}
