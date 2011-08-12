#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LAST(x) ((sizeof(x)/sizeof(*x))-1)

struct port_t {
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t bit;
};

struct port_t ledport[] = {
	{ &DDRB, &PORTB, PB0 },
	{ &DDRB, &PORTB, PB1 },
	{ &DDRB, &PORTB, PB2 },
	{ &DDRB, &PORTB, PB3 },
};

static uint8_t led[10][2] = {
	{ 0, 1 },
	{ 1, 0 },
	{ 0, 2 },
	{ 2, 0 },
	{ 0, 3 },
	{ 3, 0 },
	{ 1, 2 },
	{ 2, 1 },
	{ 1, 3 },
	{ 3, 1 },
};

static uint16_t bar_state = 0;

static void clear_bars(void) {
	for (uint8_t i=0; i<=LAST(ledport); i++) {
		*ledport[i].ddr &= ~(1<<ledport[i].bit);
	}
}

static void enable_bar(uint8_t n) {
	uint8_t high = led[n][0];
	uint8_t gnd = led[n][1];
	*ledport[high].ddr |= (1<<ledport[high].bit);
	*ledport[high].port |= (1<<ledport[high].bit);
	*ledport[gnd].ddr |= (1<<ledport[gnd].bit);
	*ledport[gnd].port &= ~(1<<ledport[gnd].bit);
}

static void set_bar(uint8_t n, uint8_t state) {
	if (state) {
		bar_state |= (1<<n);
	} else {
		bar_state &= ~(1<<n);
	}
}

static uint8_t ammo = 0;

static uint8_t ammo_btn_state = 0;

int main(void) {
	TCCR0B = (1<<CS01);
	OCR0A = 0xFA;
        TIMSK |= (1 << OCIE0A);
        sei();

	PORTD |= 1<<PD4;

	while (1) {
		uint8_t ammo_btn = ((~PIND & (1<<PD4)) != 0);
		if (ammo_btn && !ammo_btn_state) {
			set_bar( ammo++, 1 );
			_delay_ms(5);
		}
		ammo_btn_state = ammo_btn;
	}
}

SIGNAL(SIG_TIMER0_COMPA) {
	static uint8_t cur = 0;
	clear_bars();
	if (bar_state & (1<<cur)) {
		enable_bar(cur);
	}
	cur++;
	cur %= LAST(led)+1;
}
