/*
 * kbd.h
 *
 *  Created on: 7 февр. 2020 г.
 *      Author: Admin
 */

#ifndef KBD_H_
#define KBD_H_

#define KEY_READ_MASK		0x10
#define KEY_PRESSED_MASK	0x40
#define KEY_RELEASED_MASK 	0x80

#define KEY_PRESSED_FLAG_SET keyFlags == 1
#define KEY_RELEASED_FLAG_SET keyFlags == 2

#define KEY_PRESSED 1
#define KEY_NOT_PRESSED 0

#define KEY_NONE 0
#define KEY_LEFT 5
#define KEY_RIGHT 14
#define KEY_UP 6
#define KEY_DOWN 13
#define KEY_PROGRAM 3
#define KEY_OFF 11


void isr_kbd(void);
void kbd_event_handler(uint8_t context);
void register_on_key_press_callback(void (*callback)(uint8_t key, uint8_t context));

#endif /* KBD_H_ */
