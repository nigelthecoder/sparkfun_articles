/*
 * A few very simple performance tests that will run on a standard
 * Arduino Uno board as well as on the SparkFun RedBoard Artemis and 
 * RedBoard Artemis ATP.
 */

 
void setup() 
{
  // we're going to print stuff out so get the serial interface up
  Serial.begin(115200);
  Serial.println("\n\n\n\n\nBasic speed tests\n");

  // Run the assorted speed tests. 
  // These are not sophisticated, just very basic stuff to do
  // an approximate speed comparison.

  test_long_math();
  test_float_math();
  test_sqrt();

  Serial.println("\nDone\n\n");
}

void loop() 
{
  // These are not the droids you're looking for
}

void print_time(unsigned long elapsed)
{
  Serial.print("  Elapsed time: ");
  Serial.print(elapsed / 1000L);
  Serial.println(" ms");
}

// Test the time it takes to do some math on longs
void test_long_math()
{
  Serial.println("Math on long variables...");
  unsigned long start = micros();
  unsigned long num_tests = 100000L;
  volatile long a = 37;
  volatile long b = 23;
  for (unsigned long n = 0; n < num_tests; n++) {
    volatile long c = a * b;
    volatile long d = b / a;
    a = c;
    b = d;
  }

  unsigned long elapsed = micros() - start;
  print_time(elapsed);
}

// Test the time it takes to do some math on floats
void test_float_math()
{
  Serial.println("Math on float variables...");
  unsigned long start = micros();
  unsigned long num_tests = 100000L;
  volatile float a = 37;
  volatile float b = 23;
  for (unsigned long n = 0; n < num_tests; n++) {
    volatile float c = a * b;
    volatile float d = b / a;
    a = c;
    b = d;
  }

  unsigned long elapsed = micros() - start;
  print_time(elapsed);
}

// Test the time it takes to do sqrt
void test_sqrt()
{
  Serial.println("Math with sqrt...");
  unsigned long start = micros();
  unsigned long num_tests = 100000L;
  volatile float a = 37;
  volatile float b = 23;
  for (unsigned long n = 0; n < num_tests; n++) {
    volatile float c = sqrt(a);
    volatile float d = sqrt(b);
    a = c;
    b = d;
  }

  unsigned long elapsed = micros() - start;
  print_time(elapsed);
}
