/*
 * pwm.cpp
 *
 * Created: 11/6/2014 3:35:19 PM
 *  Author: Fake Name
 */


#include <asf.h>
#include "board/board.hpp"
#include "print/print.hpp"
#include "pwm/pwm.hpp"

void PWM_Handler(void)
{
	ioport_toggle_pin_level(LED_2);

	// Finally, clear the interrupt.
	pwm_channel_get_interrupt_status(PWM);
}

void setupPwm(void)
{


}

