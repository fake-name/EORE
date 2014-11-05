/**
 * \file
 *
 * \brief User board configuration template
 *
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define BOARD_FREQ_SLCK_XTAL 32768
#define BOARD_FREQ_SLCK_BYPASS BOARD_FREQ_SLCK_XTAL

#define BOARD_FREQ_MAINCK_XTAL 12000000
#define BOARD_FREQ_MAINCK_BYPASS BOARD_FREQ_MAINCK_XTAL

#define BOARD_OSC_STARTUP_US   15625


// Pin definitions
#define LED_1           IOPORT_CREATE_PIN(PIOA, 7)
#define LED_2           IOPORT_CREATE_PIN(PIOA, 8)
#define LED_3           IOPORT_CREATE_PIN(PIOA, 10)
#define LED_4           IOPORT_CREATE_PIN(PIOA, 11)

#define HEATER_ON       IOPORT_CREATE_PIN(PIOA, 5)
#define OSC_EN          IOPORT_CREATE_PIN(PIOA, 6)
#define NOISE_DIODE     IOPORT_CREATE_PIN(PIOB, 0)

#define CS_1            IOPORT_CREATE_PIN(PIOB, 1)
#define CS_2            IOPORT_CREATE_PIN(PIOB, 2)
#define CS_3            IOPORT_CREATE_PIN(PIOB, 3)
#define CS_4            IOPORT_CREATE_PIN(PIOA, 17)
#define CS_5            IOPORT_CREATE_PIN(PIOA, 18)
#define CS_6            IOPORT_CREATE_PIN(PIOA, 19)
#define CS_7            IOPORT_CREATE_PIN(PIOA, 12)

#define SWITCH_2        IOPORT_CREATE_PIN(PIOB, 1)
#define SWITCH_3        IOPORT_CREATE_PIN(PIOB, 2)
#define SWITCH_4        IOPORT_CREATE_PIN(PIOB, 3)


#define DEVICE_SCL      IOPORT_CREATE_PIN(PIOA, 4)
#define DEVICE_SDA      IOPORT_CREATE_PIN(PIOA, 3)
#define DEVICE_TWI      TWI0


#define DRV_1_ADDR  (0xA0 | 0 << 1)
#define DRV_2_ADDR  (0xA0 | 1 << 1)
#define DRV_3_ADDR  (0xA0 | 2 << 1)
#define DRV_4_ADDR  (0xA0 | 3 << 1)



#endif // CONF_BOARD_H
