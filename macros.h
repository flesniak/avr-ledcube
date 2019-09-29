#ifndef MACROS_H
#define MACROS_H

#define LED_OFF(x,y,z) (currentImage[8*z+y] |= (1<<x))
#define LED_ON(x,y,z) (currentImage[8*z+y] &= ~(1<<x))

#define BACKLIGHT_OFF (PORTB |= (1 << 0))
#define BACKLIGHT_ON (PORTB &= ~(1 << 0))

#define RCK_ON (PORTB |= (1 << 2))
#define RCK_OFF (PORTB &= ~(1 << 2))

#define BUTTON_L (!(PIND & (1<<PD3)))
#define BUTTON_R (!(PIND & (1<<PD2)))

#define WAIT_UART_READY while(!(UCSRA & (1 << UDRE)))

typedef unsigned char u8;
typedef unsigned short u16;

#endif //MACROS_H
