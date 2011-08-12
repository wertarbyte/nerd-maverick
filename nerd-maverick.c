#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LAST(x) ((sizeof(x)/sizeof(*x))-1)

struct bar_t {
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t bit;
};

struct bar_t bar[] = {
	{ &DDRB, &PORTB, PB0 },
	{ &DDRB, &PORTB, PB1 },
	{ &DDRB, &PORTB, PB2 },
	{ &DDRB, &PORTB, PB3 },
	{ &DDRB, &PORTB, PB4 },
	{ &DDRB, &PORTB, PB5 },
	{ &DDRB, &PORTB, PB6 },
	{ &DDRB, &PORTB, PB7 },
	{ &DDRD, &PORTD, PD0 },
	{ &DDRD, &PORTD, PD1 },
};

static void clear_bars(void) {
	for (uint8_t i=0; i<=LAST(bar); i++) {
		*bar[i].port &= ~(1<<bar[i].bit);
	}
}

int main(void) {
	for (uint8_t i=0; i<=LAST(bar); i++) {
		*bar[i].ddr |= (1<<bar[i].bit);
	}
	int8_t dir = -1;
	uint8_t n = 0;
	while (1) {
		clear_bars();
		*bar[n].port |= (1<<bar[n].bit);
		if (n == 0 || n == LAST(bar)) {
			dir *= -1;
		}
		n += dir;
		_delay_ms(200);
	}
}
