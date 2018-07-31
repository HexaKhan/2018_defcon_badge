#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void setupPins(){
  DDRB = ~(1 << PB5);       // Set all pins as output except PB5 (button)
  PORTB = (1 << PB5);    		// Enable pullup resistor
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
	PORTB = currentLED | (1 << PB5);
	int delayTime = 50;
	int delayIncrement = 10;
	int maxDelay = 500;
	int buttonReleased = 0;
	int flashSpeed = 250;
	int flashCount = 3;
	while (1) {
		delay_ms(delayTime);
		if ((currentLED << 1) == (1 << PB5)){		// If currentLED is out of the LED range
			currentLED = (1 << PB0);
		}
		else {
			currentLED <<= 1;
		}
		PORTB = currentLED | (1 << PB5);		// Light the LED

    if (delayTime >= 300){
      delayIncrement = 30;
    }

		if (buttonReleased){									// Button has already been released, check for winner or increment delay time

			if (delayTime > maxDelay){										// Winner Winner Chicken Dinner! Flash the winner
				delay_ms(maxDelay);
				for (int i = 0; i < flashCount*2; i++){
					PORTB ^= currentLED;	//	Toggle LED
					delay_ms(flashSpeed);
				}
        PORTB = (1 << PB5);     // Ensure LEDs are turned off
				break;									//	Break out of runRoulette
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
  PORTB = (1 << PB5);
  for (int x = 0; x < 2; x++){
    for (int i = 0; i < 5; i++){
      PORTB ^= (1 << i);
      delay_ms(delay_time);
    }
  }
}

void randomAppearPattern(int delay_time){
  PORTB = (1 << PB5);
  int appear[5] = {PB3,PB1,PB4,PB2,PB0};
  delay_ms(delay_time);
  for (int i = 0; i < 5; i++){
    PORTB |= (1 << appear[i]);
    delay_ms(delay_time);
  }
}

void randomClearPattern(int delay_time){
  PORTB = 0b00111111;
  int clear[5] = {PB2,PB1,PB4,PB0,PB3};
  for (int i = 0; i < 5; i++){
    PORTB &= ~(1 << clear[i]);
    delay_ms(delay_time);
  }
}

void flashLightsPattern(int delay_time){
  PORTB = 0b00111111;
  delay_ms(delay_time);
  PORTB = (1 << PB5);
  delay_ms(delay_time);
}

void runFunMode(){
  spinClearPattern(500);
  randomAppearPattern(1000);
  randomClearPattern(1000);
  flashLightsPattern(500);
  flashLightsPattern(500);
  flashLightsPattern(500);
}

int main(void) {
  setupPins();
  setupTimer();
  int funMode = 0;
  if (buttonPressed()){   // If button is held on start, do special effects while idle
    funMode = 1;
  }
  while (1){
    if (funMode){
      runFunMode();
    }
    else {
      continue;
    }
  }
}
