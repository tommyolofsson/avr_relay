#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define USART_BAUDRATE (9600)
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


union message {
        uint8_t byte;
        struct {
                /* 'p' happens to clear all, while for example 'x'
                 * sets relay 1.
                 */
                /* LSB (from PC pow)*/
                int relay4:1;
                int relay3:1;
                int relay2:1;
                int relay1:1;
                int do_set:1;
                int _ignored:3;
                /* MSB */
        } flags;
};

_Static_assert(sizeof(union message) == 1, "Padding in member 'flags'.");

struct ring_buf {
        uint8_t start;
        uint8_t end;
        union message buf[255];
};


static volatile struct ring_buf recv_buf;


static void setup(void)
{
        /* USART. */
	UCSR0A |= 0b00000000;
	UCSR0C |= 0b00000110;
	UCSR0B  = 0b10011000;

	UBRR0H = (BAUD_PRESCALE >> 8) & 0xff;
	UBRR0L = BAUD_PRESCALE & 0xff;

	DDRD &= ~(1 << 0);
	DDRD |= (1 << 1);

        /* Relay outputs */
        DDRC |= ((1 << 2) |
                 (1 << 3) |
                 (1 << 4) |
                 (1 << 5));
        /* Relays starts off (active low). */
        PORTC |= ((1 << 2) |
                  (1 << 3) |
                  (1 << 4) |
                  (1 << 5));

        /* TODO: Buttons */

        /* TODO: AD */

        sei();
}


ISR(USART_RX_vect)
{
	uint8_t recv;

	recv = UDR0;
	recv_buf.buf[recv_buf.end].byte = recv;
        recv_buf.end++;
}


/* Set a number of pins on the same port to the state specified. */
static inline void pin_set(volatile uint8_t *port, uint8_t pin_mask, uint8_t on)
{
        if (on)
                *port |= pin_mask;
        else
                *port &= ~pin_mask;
}
#define PINS_SET(port, pin_mask, on) pin_set(&port, pin_mask, on)
#define PIN_SET(port, pin, on) pin_set(&port, (1 << pin), on)


int main(void)
{
        setup();

        for (;;) {
                /* Handle incoming data. */
                while (recv_buf.start != recv_buf.end) {
                        union message msg;

                        msg = recv_buf.buf[recv_buf.start++];
                        if (msg.flags.do_set) {
                                /* Switch outputs according to command. */
                                PIN_SET(PORTC, 2, !(msg.flags.relay1));
                                PIN_SET(PORTC, 3, !(msg.flags.relay2));
                                PIN_SET(PORTC, 4, !(msg.flags.relay3));
                                PIN_SET(PORTC, 5, !(msg.flags.relay4));
                        } else {
                                /* Just respond with current state. */
                        }
                        /* Wait until previous send is completed. */
                        while ((UCSR0A & (1 << UDRE0)) == 0) ;
                        UDR0 = msg.byte;
                }
        }

        return 1;
}
