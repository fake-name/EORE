/*
 * spi.hpp
 *
 * Created: 11/6/2014 3:35:32 PM
 *  Author: Fake Name
 */


#ifndef BOARD_H_
#define BOARD_H_

// Convenience stuff for controling bits
#define enable(x)  ioport_set_pin_level(x, 1);
#define disable(x) ioport_set_pin_level(x, 0);

// Scaffolding, values not verified, just taken from the PCB

#define MAIN_SWITCH_MAIN_ANTENNA        0
#define MAIN_SWITCH_SWITCHED_TONE_IN    1
#define MAIN_SWITCH_RFI_ANTENNA         2
#define MAIN_SWITCH_AUX_TONE_IN         3
#define MAIN_SWITCH_TERMINATION         4
#define MAIN_SWITCH_NOISE_DIODE         5

#define TONE_SWITCH_SWR_COUPLER         0
#define TONE_SWITCH_PASS_THROUGH        1

#define MAIN_SWITCH                     0
#define TONE_SWITCH                     1

// This is the local attenuator on the control board.
#define ATTENUATOR_REF_OSC              0

#define ATTENUATOR_NOISE_DIODE          1
#define ATTENUATOR_AMP_CHAIN            2
#define ATTENUATOR_SWR_TONE             3
#define ATTENUATOR_SWITCHED_TONE        4
#define ATTENUATOR_AUX_TONE             5


#endif /* BOARD_H_ */