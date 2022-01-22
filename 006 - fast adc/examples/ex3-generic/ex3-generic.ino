/*
 * Example 3 - Generic fast ADC mechanism
 * 
 * This example shows you how to derive from the FastAdc class to sample a single
 * analog port (A0) as fast as possible and a few other analog inputs occasionally.
 * It also shows how you can process the data as it comes in from each sample. For this
 * example we compute a peak value
 * 
 * For this example we toggle a digital output pin each time we get a sample so we can see
 * the sample rate on an oscilloscope.
 *
 */

#include "fast_adc.h" 

// Define the list of ports that we want to read as fast as possible.
// We only have one in this example.
uint8_t fast_ports[] = {A0};
#define NUM_FAST_PORTS sizeof(fast_ports) / sizeof(uint8_t)

// Define the list of slow ports that we cycle through after the fast
// inputs have been read
uint8_t slow_ports[] = {A1, A2, A3};
#define NUM_SLOW_PORTS sizeof(slow_ports) / sizeof(uint8_t)

// Define a digital port to use to measure timing with the scope.
#define ISR_TIMING_PIN 2 // TBD replace with dorect port i/o

// declare our class that derives from FastAdc and lets us process the samples as they are taken
class MyAdc : public FastAdc
{
public:
  MyAdc()
  : FastAdc(fast_ports, NUM_FAST_PORTS, slow_ports, NUM_SLOW_PORTS)
  , m_peak(0)
  {
  }

  // Overload the fast conversion completion function so we can process the
  // samples as they are taken.
  // This is ISR code so keep it short.
  // This example just looks for a peak value and toggles an output port
  // bit so we can see the performace on a scope
  virtual void onFastUpdate()
  {
    // show this on the scope
    digitalWrite(ISR_TIMING_PIN, HIGH);

    // get the sample for A0
    uint16_t s = m_adc_samples[0];

    // update our peak value
    if (s > m_peak) {
      m_peak = s;
    }

    // show that we're done on the scope
    digitalWrite(ISR_TIMING_PIN, LOW);
  }

  // Get our peak value.
  // Make sure this is safe to call from foreground
  uint16_t getPeak()
  {
    uint16_t p;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      p = m_peak;
    }
    return p;
  }

  // Reset the peak value
  void resetPeak()
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      m_peak = 0;
    }
  }

private:
  uint16_t m_peak;
};

// Create the FastADC object that will sample the ports
MyAdc my_adc;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n\nFast ADC demo\n\n");

  // set up the timing pin
  pinMode(ISR_TIMING_PIN, OUTPUT);
  digitalWrite(ISR_TIMING_PIN, LOW);

  // start the ADC conversions
  my_adc.begin();

}

// convert ADC reading to voltage in millivolts
uint16_t stomv(uint16_t sample)
{
  return map(sample, 0, 1023, 0, 5000);
}

uint32_t g_cycle = 0;
void loop()
{
  // Print out our samples and the calculated peak occasionally.
  char buf[80];
  sprintf(buf, "A0: %4d mV, Peak: %4d mV, A1: %4d mV, A2: %4d mV, A3: %4d mV",
      stomv(my_adc.sample(A0)),
      stomv(my_adc.getPeak()),
      stomv(my_adc.sample(A1)),
      stomv(my_adc.sample(A2)),
      stomv(my_adc.sample(A3)));
  Serial.println(buf);

  // Just for fun we'll reset the peak every 10 cycles
  g_cycle++;
  if (g_cycle % 10 == 0) {
    my_adc.resetPeak();
    Serial.println("Peak reset");
  }

  delay(500);
}
