/*
 * Data capture using binary records over the serial port.
 * 
 * this code simulates some analog data that might be read using two ADC
 * channels.
 * 
 */

#include "Arduino.h"

// define the pattern we want to send at the start of each record.
// We are using 32 bits of all ones here but that is only one possibility.
const uint32_t START_PATTERN = 0xFFFFFFFFL;

// To avoid having to write a long-winded statement like:
// Serial.write((const uint8_t*)&v1, sizeof(v1));
// we declare a macro here so our code is a bit cleaner
#define WRITE(x) (Serial.write((const uint8_t*)&(x), sizeof(x)))
 
// A table of 256 8-bit sine values. These values are biased around 127
// so that they are all positive 8-bit values
const uint8_t sine_table[] = { 
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
  176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
  217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
  245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
  255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
  245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
  217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
  176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
  127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,
  78,75,72,70,67,64,61,59,56,54,51,49,46,44,41,39,
  37,35,33,30,28,26,25,23,21,19,18,16,15,13,12,10,
  9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,
  0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,
  9,10,12,13,15,16,18,19,21,23,25,26,28,30,33,35,
  37,39,41,44,46,49,51,54,56,59,61,64,67,70,72,75,
  78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124
};
#define SINE_TABLE_SIZE (sizeof(sine_table) / sizeof(uint8_t))
 
void setup() 
{
  // Set up the serial interface at a baud rate that is fast enough for the 
  // data records we want to send
  Serial.begin(115200);

}

uint8_t index = 0;
uint32_t start_time = micros();

void loop() 
{
  // capture the time that we take the samples.
  uint32_t now = micros();
  
  // generate a couple of analog signals to mimic what we might get
  // from reading two analog inputs with analogRead();
  // Not that our data table is only 8 bits and the ADC has a 10 bit range
  // so we shift the data 2 bits to make it use more of the range and look a bit more
  // realistic.
  uint16_t v1 = sine_table[index % SINE_TABLE_SIZE] << 2;
  uint16_t v2 = sine_table[(index + 64) % SINE_TABLE_SIZE] << 2;

  // advance the waveform generator
  index++;

  // send the data record over the serial link starting with the start sequence
  // we use Serial.write here not Serial.print as we are not sending characters
  // We also need to use the form of Serial.write that takes a pointer to a buffer and
  // a length to ensure we write the correct number of bytes
#if 0
  // The long-winded way
  Serial.write((const uint8_t*)&START_PATTERN, sizeof(START_PATTERN)); // 4 bytes
  Serial.write((const uint8_t*)&now, sizeof(now)); // 4 bytes
  Serial.write((const uint8_t*)&v1, sizeof(v1)); // 2 bytes
  Serial.write((const uint8_t*)&v2, sizeof(v2)); // 2 bytes
#else
  // Using our handy macro
  WRITE(START_PATTERN); // 4 bytes
  WRITE(now);           // 4 bytes
  WRITE(v1);            // 2 bytes
  WRITE(v2);            // 2 bytes
#endif  

  // see how long it took us to do that
  uint32_t elapsed = micros() - now;

  // wait a bit
  if (elapsed < 10000L) {
    delayMicroseconds(10000L - elapsed); // so we loop about every 10 ms
  }
  

}
