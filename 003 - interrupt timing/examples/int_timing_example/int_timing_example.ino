/*
 * This example shows how to perform timing of an external hardware event using an interrupt.
 * It compares the results of doing the timing using an interrupt with timing done using
 * calls to micros() in the foreground code.
 * An external signal source can be used to drive the input, or the squarewave generated from
 * a PWM output in the example. So, if you don't have an external source to use, just connect
 * the PWM output to both the input sampling pins and run the app.
 * Please note that the code is simplified in a few places to keep it short.
 * In particular the stats binning code assumes that the mean interval is about 1,024 us. 
 * If you use a different input source frequency, you'll need to edit the code accordingly.
 * Note also that this does not work on RedBoard Artemis as the locking scheme is different.
 * Please see the article on portable critical sections for a fix.
 * 
 * Refs: 
 * https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 * http://gammon.com.au/interrupts 
 * 
 */

#include "Arduino.h"
#include "util/atomic.h" // for ATOMIC_BLOCK macro


// Define the pins we will use for our inputs and outputs
#define FG_INPUT_PIN    3   // foreground code will do timing on this input
#define BG_INPUT_PIN    2   // Background code (on interrupt) will use this pin
#define PWM_OUTPUT_PIN  6   // PWM test signal output to this pin 
#define ISR_MONITOR_PIN 4   // Pin we can watch on a scope to see ISR timing

// Define a class to do the math required to compute mean and variance
// on a stream input of sample values.
// Ref: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
// The class update method is safe to be used inside the ISR. 
// The get... functions can be safely used in the foreground code.
class VarCalc 
{
public:
  VarCalc(const char* display_name)
  : n(0)
  , K(0)
  , Ex(0)
  , Ex2(0)
  , title(display_name)
  {      
    memset(bins, 0, sizeof(bins));
  }

  // Update the computation with a new sample value
  void update(float x)
  {
    if (n == 0) {
      K = x;
    }
    n += 1;
    Ex += x - K;
    Ex2 += (x - K) * (x - K);

    // Determine the bin to put this sample into.
    // We are cheating a lot here because we happen to know the data :)
    // The mean value is around 1024 so we bin around that
    x -= 1024; // distance from mean (approx)
    x /= 25; // bin size in microseconds --> bin number from center
    int i = 4 + (int) x; // abs bin number
    if ((i >= 0) && (i < 10)) {
      bins[i]++;
    }
  }

  float getMean()
  {
    float mean = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      mean = K + Ex / n;
    }
    return mean;
  }

  float getVariance()
  {
    float var = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      var = (Ex2 - (Ex * Ex) / n) / (n - 1);
    }
    return var;
  }

  // Copy the bin data. 
  // Your bin array MUST be >=10 elements as we don't check the size here
  void getBins(uint32_t* op_bins)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(op_bins, bins, sizeof(bins));
    }    
  }

  // Print out what we have
  void print()
  {
    Serial.print(title);
    Serial.print(": Mean: ");
    Serial.print(getMean());
    Serial.print(" us, Variance: ");
    Serial.print(getVariance());
    Serial.println();
    
    uint32_t bins[10];
    getBins(bins);
    for (uint8_t n = 0; n < 10; n++) {
      char buf[10];
      sprintf(buf, " %5ld  ", bins[n]);
      Serial.print(buf);
    }
    Serial.println();
  }

private:
  uint32_t n;
  float K;
  float Ex;
  float Ex2;
  uint32_t bins[10];
  const char* title;
};

// Create two variance calculators: one for the foreground code and one for 
// the background in the ISR. The names are only used when printing
// out the data.
VarCalc g_fgVar("Foreground");
VarCalc g_bgVar("Background");

void setup()
{
  // Set up the serial port at a fast-ish speed.
  Serial.begin(115200);
  Serial.println("\n\n\n\n\n\nTiming tests\n");
  
  // Set up the PWM squarewave output that is our test signal source
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  analogWrite(PWM_OUTPUT_PIN, 128); // 128 gives 50% duty cycle

  // Set up the two input pins 
  // Note: not really needed but I like to be clear in the code
  pinMode(FG_INPUT_PIN, INPUT);
  pinMode(BG_INPUT_PIN, INPUT);

  // To watch the ISR timing we'll toggle an output pin
  pinMode(ISR_MONITOR_PIN, OUTPUT);

  // Attach our background input pin to our interrupt
  // service routine (ISR) and trigger on the falling edge
  // of the signal.
  attachInterrupt(digitalPinToInterrupt(BG_INPUT_PIN), myISR, FALLING);
}

// A global variable to count the program loops
uint32_t g_cycle = 0;

void loop() 
{
  g_cycle++;
  
  // Measure the time between two high-to-low transitions of the 
  // input waveform.
  // Note: this is not a very practical method in a real application
  // but it shows the best we can do in measuring the waveform edges
  // in the foreground code.

  // Wait for the input to go high
  while (digitalRead(FG_INPUT_PIN) == LOW);
  
  // Wait for the input to go low
  while (digitalRead(FG_INPUT_PIN) == HIGH);

  // Start the timer
  uint32_t start = micros();

  // Wait for the input to go high
  while (digitalRead(FG_INPUT_PIN) == LOW);

  // Wait for the input to go low again
  while (digitalRead(FG_INPUT_PIN) == HIGH);

  // Measure the elapsed time
  uint32_t interval = micros() - start;

  // Update the variance calculation
  g_fgVar.update(interval);

  // Don't print the output very often so we can accumulate a few samples each time
  // and not flood the Serial Monitor so fast we can't read it.
  if ((g_cycle % 500) == 0) {

    // show what we've got so far
    g_fgVar.print();
    g_bgVar.print();    
    Serial.println();
  }

}

// variable to keep track of the previous ISR time
uint32_t g_prev_edge_time = 0;

// Our ISR which gets called when the background input pin changes state
// from high to low.
void myISR()
{
  // Get the time of the falling edge of the input signal
  uint32_t edge_time = micros();
  
  // We toggle an output pin here so we can see it on the scope and use
  // the high time to measure how long we are inside the ISR.
  digitalWrite(ISR_MONITOR_PIN, HIGH); // mark the start of the ISR

  if (g_prev_edge_time != 0) {
    // Compute the elapsed time since the previous input edge
    uint32_t elapsed = edge_time - g_prev_edge_time;

    // update the computation
    g_bgVar.update(elapsed);
  }

  // Update the previous edge time
  g_prev_edge_time = edge_time;

  // toggle the scope pin again just before we exit the ISR
  digitalWrite(ISR_MONITOR_PIN, LOW); // mark the end of the ISR
  
}
