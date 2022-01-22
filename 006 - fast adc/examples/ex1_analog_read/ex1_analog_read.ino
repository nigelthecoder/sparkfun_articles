/*
 * Example 1 - analogRead speed test
 * 
 * This doesn't need any wiring. It just reads the A0 input.
 * 
 * For the curios we toggle a port output bit each time we read a 
 * sample, so you can see the sample interval on a scope.
 */

#define SAMPLE_TIMING_PIN 2

void setup() 
{
  // Set up the serial output 
  Serial.begin(115200);
  Serial.println("\n\n\nanalogRead timing tests...\n");
  
  // Setup the A0 pin for input
  pinMode(A0, INPUT);

  // scope monitor pin
  pinMode(SAMPLE_TIMING_PIN, OUTPUT);

}

void loop() 
{
  // We will do a series of analogRead calls and then
  // see how long that took
  const uint16_t num_reads = 10000;

  // start the timer
  uint32_t start = micros();

  // do some reads
  for (uint16_t n = 0; n < num_reads; n++) {
    digitalWrite(SAMPLE_TIMING_PIN, HIGH);
    uint16_t aval = analogRead(A0);
    digitalWrite(SAMPLE_TIMING_PIN, LOW);
  }

  // measure the time 
  uint32_t elapsed = micros() - start;

  // print out the results
  Serial.print(num_reads);
  Serial.print(" analogRead calls took: ");
  Serial.print(elapsed);
  Serial.print(" us. ");
  Serial.print(elapsed / num_reads);
  Serial.println(" us per call.");
}
