/*
 * Test disabling interrupts to establish a critical section.
 * 
 * IMPORTANT: Connect pin 6 to pin 2 so that the squarewave on pin 6 can cause
 * interrupts on pin 2.
 * 
 * V2: using critical section macros for each platform.
 *     Also changed to using an external interrupt driven by the PWM generator
 *     so we don't depend on the behavior of millis()
 *     
 * Apollo 3 critical section ref: 
 * https://github.com/sparkfun/AmbiqSuiteSDK/blob/master/mcu/apollo3/regs/am_reg_macros.h
 * 
 */

#include "critical_section.h"



// Set up the pins
#define BG_INPUT_PIN    2   // Background code (on interrupt) will use this pin
#define PWM_OUTPUT_PIN  6   // PWM test signal output to this pin 

// A couple of arbitrary values we use in the ISR
#define ISR_VAL_1 0x11223344L
#define ISR_VAL_2 0x55667788L

void setup() 
{
  Serial.begin(115200);
  Serial.println("Critical section tests");

  // Set up the PWM squarewave output that is our test signal source
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  analogWrite(PWM_OUTPUT_PIN, 128); // 128 gives 50% duty cycle

  // Set up the input pin
  pinMode(BG_INPUT_PIN, INPUT);

  // Attach our background input pin to our interrupt
  // service routine (ISR) and trigger on the falling edge
  // of the signal.
  attachInterrupt(digitalPinToInterrupt(BG_INPUT_PIN), myISR, FALLING);

}

// Some data values that are set by the ISR and interrogated
// by the foreground code
volatile uint32_t g_isr_count = 0;
volatile uint32_t g_isr_value_a = 0;
volatile uint32_t g_isr_value_b = 0;

// Our ISR which gets called when the background input pin changes state
// from high to low.
void myISR()
{
  // increment the interrupt counter
  g_isr_count++;

  // We alternate writing one of two known values to the
  // global values.
  if (g_isr_count & 0x01) {
    g_isr_value_a = ISR_VAL_1;
    g_isr_value_b = ISR_VAL_1;
  } else {
    g_isr_value_a = ISR_VAL_2;
    g_isr_value_b = ISR_VAL_2;
  }
}

void loop()
{
  Serial.println("\n\n\n\n");
  test_1(); // make sure the ISR is working (You have it wired up right)
  test_2();  
  test_3();
  test_4();
  test_1(); // just to be sure we didn't leave interrupts disabled
  
  delay(3000);
}

void test_1()
{
  Serial.println("Test 1 (ISR running)...");
  delay(100); // allow the ISR to change the values
  uint32_t vala = g_isr_value_a;
  uint32_t valb = g_isr_value_b;
  uint32_t cnt = g_isr_count;
  if ((vala == 0) || (valb == 0) || (cnt == 0)) {
    Serial.println("Failed. ISR not running? Check wiring please.");
    delay(500);  
  } else {
    Serial.println("OK");
  }
}

void test_2()
{
  Serial.println("Test 2 (no locking, expecting conflicts)...");
  uint32_t start = millis();
  uint32_t errs = 0;
  while ((millis() - start) < 10000) {
    uint32_t vala = g_isr_value_a;
    uint32_t valb = g_isr_value_b;
    uint32_t cnt = g_isr_count;
    if (((vala != ISR_VAL_1) && (vala != ISR_VAL_2))
    || ((valb != ISR_VAL_1) && (valb != ISR_VAL_2))
    || (vala != valb)) {
      errs++;
      Serial.print("Error ");
      Serial.print(errs);
      Serial.print(" (expected) at count: ");
      Serial.print(cnt);  
      Serial.print(", Value A: ");
      Serial.print(String(vala, HEX));
      Serial.print(", Value B: ");
      Serial.println(String(valb, HEX));
      delay(500);  
      if (errs >= 5 ) break; // we've seen enough
    }
  }
  if (errs != 0) Serial.println("OK");
}

void test_3()
{
  Serial.println("Test 3 (critical sections, look for conflict)...");
  uint32_t start = millis();
  uint32_t errs = 0;
  while ((millis() - start) < 10000) {
    uint32_t vala;
    uint32_t valb;
    uint32_t cnt;

    // Use critical section to guard the values from the ISR
    // NOTE: You MUST use both the CS_BEGIN and CS_END macros.
    CS_BEGIN
    vala = g_isr_value_a;
    valb = g_isr_value_b;
    cnt = g_isr_count;
    CS_END

    // Verify we got what we expected
    if (((vala != ISR_VAL_1) && (vala != ISR_VAL_2))
    || ((valb != ISR_VAL_1) && (valb != ISR_VAL_2))
    || (vala != valb)) {
      errs++;
      Serial.print("Error ");
      Serial.print(errs);
      Serial.print(" (NOT expected) at count: ");
      Serial.print(cnt);  
      Serial.print(", Value A: ");
      Serial.print(String(vala, HEX));
      Serial.print(", Value B: ");
      Serial.println(String(valb, HEX));
      delay(500);  
    }
  }
  if (errs == 0) Serial.println("OK");
  
}

void test_4()
{
  Serial.println("Test 4 (CS lock object, look for conflict)...");
  uint32_t start = millis();
  uint32_t errs = 0;
  while ((millis() - start) < 10000) {
    uint32_t vala;
    uint32_t valb;
    uint32_t cnt;

    // Use critical section scope block to guard the values from the ISR
    { // begin lock scope
      CS_LOCK // take the lock
      vala = g_isr_value_a;
      valb = g_isr_value_b;
      cnt = g_isr_count;
    } // end of lock scope
 
    // Verify we got what we expected
    if (((vala != ISR_VAL_1) && (vala != ISR_VAL_2))
    || ((valb != ISR_VAL_1) && (valb != ISR_VAL_2))
    || (vala != valb)) {
      errs++;
      Serial.print("Error ");
      Serial.print(errs);
      Serial.print(" (NOT expected) at count: ");
      Serial.print(cnt);  
      Serial.print(", Value A: ");
      Serial.print(String(vala, HEX));
      Serial.print(", Value B: ");
      Serial.println(String(valb, HEX));
      delay(500);  
    }
    if (errs >= 5) break; // thanks for trying :(
  }
  if (errs == 0) Serial.println("OK");
  
}
