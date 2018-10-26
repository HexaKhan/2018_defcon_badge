#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Global Variables
int funMode = 0;

// After testing, found that the lights are not sequential to PB bits on the board
// Super hacky fix to light them up in the right order
uint8_t bitSwapMachine(uint8_t lightBits){
  uint8_t finalByte = lightBits;
  if ((1 << PB2) & lightBits){  // If PB2 bit is 1, set PB3 to 1 (else 0)
    finalByte |= (1 << PB3);
  }
  else {
    finalByte &= ~(1 << PB3);
  }
  if ((1 << PB3) & lightBits){  // If PB3 bit is 1, set PB4 to 1 (else 0)
    finalByte |= (1 << PB4);
  }
  else {
    finalByte &= ~(1 << PB4);
  }
  if ((1 << PB4) & lightBits){  // If PB4 bit is 1, set PB2 to 1 (else 0)
    finalByte |= (1 << PB2);
  }
  else {
    finalByte &= ~(1 << PB2);
  }
  return finalByte;
}

// Related to hacky PB bit fix, but also flips the light bits as another issue was found
// The board has the lights attached to VCC vs. ground (my bad), flip the bits since 1 = off, 0 = on
uint8_t bitFixMachine(uint8_t lightBits){
  return bitSwapMachine(lightBits) ^ 0b00011111;  // Flip the lights bits
}

void setupPins(){
  DDRB = ~(1 << PB5);       // Set all pins as output except PB5 (button)
  PORTB = bitFixMachine((1 << PB5));    		// Enable pullup resistor
}

void setupTimer(){
  TCCR0B |= (1 << CS01) | (1 << CS00); // Enable /64 pre-scaler
  TIMSK |= (1 << TOIE0);               // Enable overflow interrupt
  sei();
}

int buttonPressed() {
  if (!(PINB & (1 << PB5))) {   // If PB5 is NOT 1 (since were pulling high)
    return 1;
  }
  else {
    return 0;
  }
}

void delay_ms(int count) {			// Dumbest thing, you can't pass variables directly to _delay_ms...
  while(count--) {
    _delay_ms(1);
  }
}

void runRoulette(uint8_t startPin){
  uint8_t currentLED = startPin;
  PORTB = bitFixMachine(currentLED) | (1 << PB5);
  int delayTime = 50;
  int delayIncrement = 10;
  int maxDelay = 500;
  int buttonReleased = 0;
  int flashSpeed = 250;
  int flashCount = 3;
  while (1) {
    delay_ms(delayTime);
    if ((currentLED << 1) == (1 << PB5)){   // If currentLED is out of the LED range
      currentLED = (1 << PB0);
    }
    else {
      currentLED <<= 1;
    }
    PORTB = bitFixMachine(currentLED) | (1 << PB5);   // Light the LED

    if (delayTime >= 250){
      delayIncrement = 30;
    }

    if (delayTime >= 400){
      delayIncrement = 50;
    }

    if (buttonReleased){         // Button has already been released, check for winner or increment delay time

      if (delayTime > maxDelay){       // Winner Winner Chicken Dinner! Flash the winner
        delay_ms(delayTime*2);
        for (int i = 0; i < flashCount*2; i++){
          PORTB ^= bitSwapMachine(currentLED);  //  Toggle LED
          delay_ms(flashSpeed);
        }
        PORTB = bitFixMachine((1 << PB5));     // Ensure LEDs are turned off
        if (funMode){
          delay_ms(750);       // Delay helps with special effects (or else they start too soon)
        }
        break;                  //  Break out of runRoulette
      }
      else{
        delayTime += delayIncrement;
      }
    }
    else {
      if (!buttonPressed()){
        buttonReleased = 1;
      }
    }
  }
}

//Variables for interrupt
volatile int timesPressed = 0;
volatile int programRunning = 0;
volatile uint8_t startingPin = (1 << PB0);

ISR(TIMER0_OVF_vect){
  if (buttonPressed() && !programRunning){  // Checking for program running may not be needed at all..
    timesPressed++;
    if (timesPressed >= 3){               // 64 pre-scaler at 1Mhz overflows every 0.016384s, so ~50ms to trigger
      programRunning = 1;
      runRoulette(startingPin);
      timesPressed = 0;
      programRunning = 0;
      startingPin <<= 2;                   // increment the starting pin by 2 to give some sort of extra randomness
      if (startingPin >= (1 << PB5)){
        startingPin >>= 5;
      }
    }
  }
  else {
    timesPressed = 0;   // If button isn't 'pressed', it's not pressed or bouncing
  }
}

void spinClearPattern(int delay_time){
  PORTB = bitFixMachine((1 << PB5));
  for (int x = 0; x < 2; x++){
    for (int i = 0; i < 5; i++){
      PORTB ^= bitSwapMachine((1 << i));
      delay_ms(delay_time);
    }
  }
}

void randomAppearPattern(int delay_time){
  PORTB = bitFixMachine((1 << PB5));
  int appear[5] = {PB3,PB1,PB4,PB2,PB0};
  delay_ms(delay_time);
  for (int i = 0; i < 5; i++){
    PORTB ^= bitSwapMachine((1 << appear[i]));
    delay_ms(delay_time);
  }
}

void randomClearPattern(int delay_time){
  PORTB = bitFixMachine(0b00111111);
  int clear[5] = {PB2,PB1,PB4,PB0,PB3};
  for (int i = 0; i < 5; i++){
    PORTB |= bitSwapMachine((1 << clear[i]) | (1 << PB5));
    delay_ms(delay_time);
  }
}

void flashLightsPattern(int delay_time){
  PORTB = bitFixMachine(0b00111111);
  delay_ms(delay_time);
  PORTB = bitFixMachine((1 << PB5));
  delay_ms(delay_time);
}

void fanPattern(int delay_time){
  uint8_t pattern[4] = {0b00000000,0b00000100,0b00001110,0b00011111};
  for (int i = 0; i < 4; i++){
    PORTB = bitFixMachine(pattern[i] | (1 << PB5));
    delay_ms(delay_time);
  }
  for (int i = 3 - 1; i >= 1; i--){
    PORTB = bitFixMachine(pattern[i] | (1 << PB5));
    delay_ms(delay_time);
  }
  PORTB = bitFixMachine((1 << PB5));
}

void triangleSpinPattern(int delay_time){
  uint8_t pattern[5] = {0b00010110,0b00001011,0b0010101,0b00011010,0b00001101};
  for (int i = 0; i < 4; i++){
    PORTB = bitFixMachine(pattern[i] | (1 << PB5));
    delay_ms(delay_time);
  }
}

void runFunMode(){
  spinClearPattern(500);
  randomAppearPattern(1000);
  randomClearPattern(1000);
  triangleSpinPattern(500);
  flashLightsPattern(500);
  flashLightsPattern(500);
  fanPattern(500);
  fanPattern(500);
  spinClearPattern(250);
  randomAppearPattern(500);
  randomClearPattern(500);
  triangleSpinPattern(250);
  flashLightsPattern(250);
  flashLightsPattern(250);
  fanPattern(250);
  fanPattern(250);
  spinClearPattern(125);
  randomAppearPattern(250);
  randomClearPattern(250);
  triangleSpinPattern(125);
  flashLightsPattern(125);
  flashLightsPattern(125);
  fanPattern(125);
  fanPattern(125);
}

int main(void) {
  setupPins();
  setupTimer();
  if (buttonPressed()){   // If button is held on start, do special effects while idle
    funMode = 1;
  }
  if (funMode){
    while (1){
      runFunMode();
    }
  }
  else {
    while (1){
      continue;
    }
  }
}
