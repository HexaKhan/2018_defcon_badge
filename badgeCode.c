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

void runRoulette(){
	uint8_t currentLED = (1 << PB0);
	PORTB = currentLED | (1 << PB0);
	int delayTime = 50;
	int delayIncrement = 10;
	int maxDelay = 500
	int buttonReleased = 0;
	int flashSpeed = 250;
	int flashCount = 3;
	while (1) {
		_delay_ms(delayTime);
		if ((currentLED << 1) == (1 << PB5)){		// If currentLED is out of the LED range
			currentLED = (1 << PB0);
		}
		else {
			currentLED <<= 1;
		}
		PORTB = currentLED | (1 << PB5);		// Light the LED

		if buttonReleased{									// Button has already been released, check for winner or increment delay time
			if !(delayTime+delayIncrement >= maxDelay){
				delayTime += delayIncrement;
			}
			else {										// Winner Winner Chicken Dinner! Flash the winner
				_delay_ms(maxDelay);
				for (i = 0; i < flashCount*2; i++){
					PORTB ^= currentLED;	//	Toggle LED
					_delay_ms(flashSpeed);
				}
				break;									//	Break out of runRoulette
			}
		}
		else {
			if !buttonPressed(){
				buttonReleased = 1;
			}
		}
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
