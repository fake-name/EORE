/*
 * pwm.cpp
 *
 * Created: 11/6/2014 3:35:19 PM
 *  Author: Fake Name
 */


#include <asf.h>
#include "board/board.hpp"
#include "print/print.hpp"
#include "tmp/tmp100.hpp"
#include "pwm/pwm.hpp"


// #define DEBUG_PR(x ...)  // Default to NO debug
#define DEBUG_PR(x ...) debugUnique(x)    // UnComment for Debug

volatile double pwm_setpoint;
volatile double current_temp;
volatile SPid pwm_state;
pwm_channel_t pwm_channel_instance;


double UpdatePID(volatile SPid * pid, double error, double position)
{
	double pTerm,
	dTerm, iTerm;
	pTerm = pid->pGain * error;
	// calculate the proportional term
	// calculate the integral state with appropriate limiting
	pid->iState += error;
	if (pid->iState > pid->iMax)
	{
		pid->iState = pid->iMax;
	}
	else if (pid->iState < pid->iMin)
	{
		pid->iState = pid->iMin;
	}
	iTerm = pid->iGain * pid->iState; // calculate the integral term
	dTerm = pid->dGain * (position - pid->dState);
	pid->dState = position;
	return pTerm + iTerm - dTerm;
}

void setupPwm(void)
{
	pwm_setpoint = 0;
	pwm_state.pGain = 100;
	pwm_state.iGain = 2;
	pwm_state.dGain = 10;

	// PWM Output has a range of 0-999
	pwm_state.iMax = 999;
	pwm_state.iMin = 0;


	pwm_channel_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel_instance.ul_period = 1000;
	pwm_channel_instance.ul_duty = 1000;
	pwm_channel_instance.channel = PWM_CHANNEL_2;



}

// Update the PID control loop target temperature
// setpoint is in °C.
void set_temperature(float setpoint)
{
	cpu_irq_disable();
	pwm_setpoint = (double) setpoint;
	cpu_irq_enable();
}

void set_pid_kp(float setpoint)
{
	cpu_irq_disable();
	pwm_state.pGain = (double) setpoint;
	cpu_irq_enable();
}
void set_pid_ki(float setpoint)
{
	cpu_irq_disable();
	pwm_state.iGain = (double) setpoint;
	cpu_irq_enable();
}
void set_pid_kd(float setpoint)
{
	cpu_irq_disable();
	pwm_state.dGain = (double) setpoint;
	cpu_irq_enable();
}




float getTemperatureSetpoint(void)
{
	return (float) pwm_setpoint;
}

float getTemperature(void)
{
	return (float) current_temp;
}

float get_pid_kp(void)
{
	return (float) pwm_state.pGain;
}
float get_pid_ki(void)
{
	return (float) pwm_state.iGain;
}
float get_pid_kd(void)
{
	return (float) pwm_state.dGain;
}





// Hooked up to the interrupt through some bizarre forward-define
// magic in the ASF headers.
void PWM_Handler(void)
{
	ioport_toggle_pin_level(LED_3);

	ioport_set_pin_level(LED_2, 1);
	uint16_t tempval;

	tempval = 0;
	read_temp(1, &tempval);


	double calc = ((int16_t) tempval) >> 4;
	calc = 0.0628 * calc;

	current_temp = calc;

	double err = pwm_setpoint - calc;
	double ret;
	ret = UpdatePID(&pwm_state, err, calc);

	int16_t newPwm = (int16_t)ret;

	if (newPwm < 0)
	{
		newPwm = 0;
	}
	if (newPwm > 1000)
	{
		newPwm = 1000;
	}


	// The PWM output is inverted (high is off, low is on)
	// so we invert the control signal.
	newPwm = 1000-newPwm;

	// Disable the PWM output if the temperature is 0
	if (pwm_setpoint <= 0)
	{
		newPwm = 1000;
	}

	pwm_channel_update_duty(PWM, &pwm_channel_instance, newPwm);

	// DEBUG_PR("TMP reading: %i, %f. PID: %f, %i, err: %f. PWM: %i", tempval, calc, ret, (uint16_t)ret, err, newPwm);
	// DEBUG_PR("%f,%f,%f,%i", calc, ret, err, newPwm);

	// Finally, clear the interrupt.
	pwm_channel_get_interrupt_status(PWM);
	ioport_set_pin_level(LED_2, 0);
}


