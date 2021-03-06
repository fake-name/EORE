/*
 * spi.hpp
 *
 * Created: 11/6/2014 3:35:32 PM
 *  Author: Fake Name
 */


#ifndef PWM_H_
#define PWM_H_

void setupPwm(void);

// This will be SLOW. However, it's only firing 1 time per second, so we don't care.
typedef struct
{
	double dState; // Last position input
	double iState; // Integrator state

	// Maximum and minimum allowable integrator state
	double iMax;
	double iMin;

	double iGain; // integral gain
	double pGain; // proportional gain
	double dGain; // derivative gain
} SPid;



void set_temperature(float setpoint);
void set_pid_kp(float setpoint);
void set_pid_ki(float setpoint);
void set_pid_kd(float setpoint);
float getTemperatureSetpoint(void);
float getTemperature(void);
float get_pid_kp(void);
float get_pid_ki(void);
float get_pid_kd(void);


#endif /* PWM_H_ */