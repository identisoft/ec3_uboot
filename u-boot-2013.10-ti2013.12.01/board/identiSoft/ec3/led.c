/*
 * Copyright (c) 2010 Texas Instruments, Inc.
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <status_led.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

/* GPIO pins for the LEDs */
#define RED_LED_FLT	53	/* GPIO1_21 */
#define GREEN_LED_RUN	54	/* GPIO1_22 */


void __led_init (led_id_t mask, int state)
{
	__led_set (mask, state);
}

void __led_toggle (led_id_t mask)
{
	int state, toggle_gpio = 0;
#ifdef STATUS_LED_BIT
	if (!toggle_gpio && STATUS_LED_BIT & mask)
		toggle_gpio = GREEN_LED_RUN;
#endif
#ifdef STATUS_LED_BIT1
	if (!toggle_gpio && STATUS_LED_BIT1 & mask)
		toggle_gpio = RED_LED_FLT;
#endif
	if (toggle_gpio) {
		if (!gpio_request(toggle_gpio, "")) {
			gpio_direction_output(toggle_gpio, 0);
			state = gpio_get_value(toggle_gpio);
			gpio_set_value(toggle_gpio, !state);
		}
	}
}

void __led_set (led_id_t mask, int state)
{
#ifdef STATUS_LED_BIT
	if (STATUS_LED_BIT & mask) {
		if (!gpio_request(GREEN_LED_RUN, "")) {
			gpio_direction_output(GREEN_LED_RUN, 0);
			gpio_set_value(GREEN_LED_RUN, !state);
		}
	}
#endif
#ifdef STATUS_LED_BIT1
	if (STATUS_LED_BIT1 & mask) {
		if (!gpio_request(RED_LED_FLT, "")) {
			gpio_direction_output(RED_LED_FLT, 0);
			gpio_set_value(RED_LED_FLT, !state);
		}
	}
#endif
}
