/*
 * Blue LED flasher
 * Flashes the LED that is on pin 13 on most Arduino boards
 * but is on pin 5 of the SparkFun RedBoard Artemis ATP
 * The LED_BUILTIN define takes care of this
 */

// The pin for the LED we want to flash
#define BLUE_LED_PIN LED_BUILTIN
 
void setup() 
{
  pinMode(BLUE_LED_PIN, OUTPUT);
}

// An array of flash times
uint16_t flash_times[] = {
  100, 200, 400, 800
};
#define NUM_FLASH_TIMES (sizeof(flash_times) / sizeof(uint16_t))

void loop() 
{
  for (uint8_t n = 0; n < NUM_FLASH_TIMES; n++) {
    uint16_t ft = flash_times[n];
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(ft);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(ft);
  }
}
