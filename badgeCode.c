#include <avr/io.h>
#include <util/delay.h>

void flashWinner(uint8_t winnerLED){
	int winFlashRate = 250;
	int flashCount = 5;
	for (int i = 0; i > flashCount; flashCount++){
		PORTB = winnerLED;
		_delay_ms(winFlashRate);
		PORTB = 0x00;
		_delay_ms(winFlashRate);
	}
}

int main(void) {
	DDRB = 0b00011111;
	uint8_t buttonBit = (1<<5);
	int delayMS = 100;
	uint8_t currentLED = (1<<4);
	PORTB = (currentLED | buttonBit);
	while (1){
		if (!(PINB & (buttonBit))){
			PORTB = ( 0b00011111 | buttonBit);
			while (!(PINB & (buttonBit))){
				_delay_ms(10);
			}
		}
		PORTB = (currentLED | buttonBit);
		_delay_ms(delayMS);
		if (currentLED == (1<<0)) {
			currentLED = (1<<4);
		} else {
			currentLED >>= 1;
		}
	}
	return 0;
}