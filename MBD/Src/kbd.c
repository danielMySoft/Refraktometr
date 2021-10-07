/*
 * kbd.c
 *
 *  Created on: 7 февр. 2020 г.
 *      Author: Admin
 */
#include "main.h"
#include "kbd.h"

static volatile uint8_t keyCode;

static void (*on_key_press_callback)(uint8_t key, uint8_t context);

void register_on_key_press_callback(void (*callback)(uint8_t key, uint8_t context))
{
	on_key_press_callback = callback;
}

inline void isr_kbd(void)
{
	static uint8_t i;
	static uint8_t keyState[2];
	uint32_t k;
	k = GPIOE->IDR;						//keyboard row and column port
	k >>= 4;
	k = k & 0x7;
//	k = (GPIOE->IDR & 0x7);
	if(k < 7)							//if any button pressed
	{
		keyCode |= k;
		keyCode = (keyCode | (i<<3));
		keyState[i] = KEY_PRESSED;
		keyCode |= KEY_PRESSED_MASK;
	}
	else keyState[i] = KEY_NOT_PRESSED;

	if (keyState[0] == KEY_NOT_PRESSED && keyState[1] == KEY_NOT_PRESSED) keyCode &= (~KEY_PRESSED_MASK);

	if (++i > 1) i = 0;

	if (i == 0) GPIOE -> BSRR = (1 << 2) | ((1 << 3) << 16);	//Scan next column
	else GPIOE -> BSRR = (1 << 3) | ((1 << 2) << 16);


}


void kbd_event_handler(uint8_t context)
{
	//uint8_t kk[10];
	static uint8_t keyFlags;
	keyFlags = keyCode >> 6;
	if (keyFlags && keyFlags < 3)		//if any key flags set
	{
		if (KEY_PRESSED_FLAG_SET)
		{
//			UART_print("KEY_PRESSED\r\n");
//			sprintf(kk,"%d\r\n", keyCode & 0x0F);
//			UART_print(kk);

			keyCode |= KEY_RELEASED_MASK;
			if (on_key_press_callback) on_key_press_callback(keyCode & 0x0F, context);
		}
		if (KEY_RELEASED_FLAG_SET)
		{
//			UART_print("KEY_RELEASED\r\n");
			keyCode = KEY_READ_MASK;
		}
	}
}
