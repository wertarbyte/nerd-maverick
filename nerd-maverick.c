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
	{ &DDRB, &PORTB, PB0 }, /* RED */
	{ &DDRB, &PORTB, PB1 }, /* YELLOW */
	{ &DDRB, &PORTB, PB2 }, /* GREEN */
	{ &DDRB, &PORTB, PB3 }, /* GREY */
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
		*ledport[i].port &= ~(1<<ledport[i].bit);
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

static uint8_t next_chamber_is_loaded(void) {
	return (PIND & (1<<PD2)) == 0;
}

static uint8_t gun_is_cocked(void) {
	/* if the gun is cocked, the switch is _open_ */
	return (~PIND & (1<<PD3)) == 0;
}

static uint8_t trigger_is_pulled(void) {
	return (~PIND & (1<<PD0)) != 0;
}

static uint8_t ammo_i = 0;

static uint8_t barrel_btn_state = 0;
static uint8_t cocking_btn_state = 0;

static uint8_t current_barrel_clear = 0;

static void clear_ammo(void) {
	for (uint8_t i=0; i<6; i++) {
		set_bar(i, 0);
	}
}

static void barrel_rotated(void) {
	set_bar( ammo_i, !current_barrel_clear );
	current_barrel_clear = 0;
	ammo_i++;
	ammo_i %= 6;
}

static void trigger_pulled(void) {

}

static void gun_fired(void) {
	set_bar( ammo_i, 0 );
}

int main(void) {
	TCCR0B = (1<<CS01);
	OCR0A = 0xFA;
        TIMSK |= (1 << OCIE0A);
        sei();

	PORTD |= 1<<PD0;
	PORTD |= 1<<PD3;
	PORTD |= 1<<PD4;

	for (uint8_t n=0; n<10; n++) {
		set_bar(n, 1);
		_delay_ms(1000);
	}
	_delay_ms(5000);
	for (uint8_t n=0; n<10; n++) {
		set_bar(n, 0);
		_delay_ms(1000);
	}

	set_bar(9, 1);
	while (1) {
		set_bar(8, next_chamber_is_loaded());
		if (!next_chamber_is_loaded()) {
			/* we did see a light, so the next chamber is not loaded */
			set_bar( ammo_i, 0);
			current_barrel_clear = 0;
		}

		uint8_t cocking_btn = gun_is_cocked();
		set_bar(7, cocking_btn);
		if (cocking_btn != cocking_btn_state) {
			cocking_btn_state = cocking_btn;
			if (!cocking_btn) {
				gun_fired();
			}
		}

		uint8_t barrel_btn = ((~PIND & (1<<PD4)) != 0);
		set_bar(9, barrel_btn);
		if (barrel_btn != barrel_btn_state) {
			barrel_btn_state = barrel_btn;
			if (!barrel_btn) {
				/* wait until the chamber is rotated in front of the sensor */
				_delay_ms(500);
				barrel_rotated();
			}
		}
		_delay_ms(50);
	}
}

SIGNAL(SIG_TIMER0_COMPA) {
	static uint8_t cur = 0;
	static uint8_t count = 0;
	clear_bars();
	if (bar_state & (1<<cur)) {
		enable_bar(cur);
	}
	cur++;
	cur %= LAST(led)+1;
}
