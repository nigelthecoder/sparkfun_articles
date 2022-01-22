/*
 * Example 2 - fastest possible read from A0
 * 
 * This example shows you how fast reading a single analog port can go.
 * We set up the ADC with a faster clock than normal and use an interrupt
 * to grab the results when a conversion is complete.
 * 
 * For this example we toggle a digital output pin each time we get a sample so we can see
 * the sample rate on an oscilloscope.
 * 
 * Note that ANY code inside the ISR takes time to run and this lengthens the overall
 * analog capture time. You can optimize a bit by starting the next ADC capture before
 * processing the data retrieved from the ADC. But beware that your code does not take longer to run than 
 * the ADC capture time (13 us). 
 * In any case, your code is running for a significant portion of the available CPU cycles, so this 
 * code will slow down the execution of the foreground code. No free lunch.
 * 
 */

// We use a locking mechanism from the AVR sources
#include "util/atomic.h"

// Choose the prescaler for the ADC. 16 works well and gives
// 13 us conversion times. 8 is twice as fast (6.5 us) but you'll 
// loose some conversion precision.
// NOTE: the code below will only work for 8 or 16, any other values
// will result in a prescaler value of 16
// If you want a different prescaler, look at the MCU datasheet
#define ADC_PRESCALER 16

// define the port and port bit to use for the scope output
// pin 2 is on Port D at bit 2 (0x04)
#define DIRECT_PORT   PORTD   // The port to use
#define DIRECT_DDR    DDRD    // Data direction register for the port
#define DIRECT_BIT 2          // The bit number for the pin we want
#define DIRECT_MASK (1 << DIRECT_BIT)

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n\nFast ADC demo reading A0\n\n");

  // Set up the port bit we will use to monitor the 
  // IST execution on the scope.
  DIRECT_DDR |= DIRECT_MASK; // 1 = output
  PORTD &= ~DIRECT_MASK; // low

  // Enable the ADC
  ADCSRA =  bit(ADEN);   // turn ADC on

  // Configure the ADC prescaler.
  // See ATmega328P spec section 23.4 and Table 23-5
  // /16 gives us a 1 MHz ADC clock
  // /8 gives a 2 MHz clock
  if (ADC_PRESCALER == 8) {
    // ludicrous mode
    ADCSRA |= bit(ADPS1) | bit(ADPS0);  // Prescaler of 8
  } else {
    // assume 16
    ADCSRA |= bit(ADPS2);  // Prescaler of 16
  }

  // Set up the multiplexer for the A0 input
  uint8_t index = 0; // Change this to use a port other than A0
  ADMUX = bit(REFS0) | (index & 0x07);

#if defined (__AVR_ATmega2560__)
  // Mega mux has another selector for 8..15
  ADCSRB = (index > 7) ? bit(MUX5) : 0;
#endif

  // start the first conversion with the interrupt enabled
  ADCSRA |= bit(ADSC) | bit(ADIE);

}

// This is where we will store the ADC conversion result
// that we get in the ISR.
volatile uint16_t g_adc_value = 0;

// This is our ISR which hooks into the ADC interrupt vector
ISR (ADC_vect)
{
  // for this example we aren't going to do anything here except grab the 
  // conversion from the ADC and store it in a global memory variable.
  // In a real app you'll be doing some processing of the ADC value in here.
  
  // Set the scope output pin high so we can see the timing on the scope.
  PORTD |= DIRECT_MASK; // high

  // get the conversion result
  g_adc_value = ADC;

  // This is where you might do some math on the sample etc.
  // YOUR CODE HERE

  // Start the next conversion
  ADCSRA |= bit (ADSC) | bit (ADIE);

  // Or maybe YOUR CODE HERE if you are brave enough to start the next
  // ADC conversion before you do your math etc.

  // mark the end of the ISR so we can see it on the scope
  PORTD &= ~DIRECT_MASK; // low
}

void loop()
{
  // Print out one of our samples occasionally.
  // Get the most recent sample. Note that this changes as fast as the ISR
  // gets called but we are only taking it once in a while here to show the value.
  // When we read a 16-bit value we need to be sure the ISR does not change it between us reading 
  // the low byte and the high byte. So we use an AVR lock macro here to temporarliy disable
  // interrupts while we do the read.
  uint16_t sample_value;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    sample_value = g_adc_value;
  }

  Serial.print("A0: ");
  Serial.println(sample_value);
  
  delay(500);
}
