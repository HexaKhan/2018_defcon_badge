#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void setupPins(){
  DDRB = ~(1 << PB5);       // Set all pins as output except PB5 (button)
  PORTB = (1 << PB5);    		// Enable pullup resistor
}

void setupTimer(){
  TCCR0B |= (1 << CS01) | (1 << CS00); //Enable /64 pre-scaler
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

void runRoulette(){
	currentLED = (1 << PB0);
	PORTB = (1 << PB0) | (1 << PB5);
	int revolutions = 5;
	for (int i = 0; i < revolutions; i++){
		if (currentLED << 1) == (1 << PB5){
			currentLED = (1 << PB0);
		}
		else {
			currentLED <<= 1;
		}
		PORTB = currentLED | (1 << PB5);

	}
}

volatile int timesPressed = 0;
volatile int programRunning = 0;
ISR(TIMER0_OVF_vect){
  if (buttonPressed() && !programRunning){  // Checking for program running may not be needed at all..
    timesPressed++;
    if (timesPressed >= 3){               // 64 pre-scaler at 1Mhz overflows every 0.016384s, so ~50ms to trigger
      programRunning = 1;
      runRoulette();
      timesPressed = 0;
      programRunning = 0;
    }
  }
  else {
    timesPressed = 0;   // If button isn't 'pressed', it's not pressed or bouncing
  }
}

int main(void) {
  setupPins();
  setupTimer();
  while (1){
    continue;
  }
}
