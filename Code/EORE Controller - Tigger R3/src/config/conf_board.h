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
#define LED_3           IOPORT_CREATE_PIN(PIOA, 11)
#define LED_4           IOPORT_CREATE_PIN(PIOA, 12)

#define HEATER_ON       IOPORT_CREATE_PIN(PIOA, 5)
//#define HEATER_OLD      IOPORT_CREATE_PIN(PIOA, 5)
#define OSC_EN          IOPORT_CREATE_PIN(PIOA, 9)
#define NOISE_DIODE_PS  IOPORT_CREATE_PIN(PIOB, 0)
#define VCO_SWEEPER_PS  IOPORT_CREATE_PIN(PIOA, 0)

#define CS_1            IOPORT_CREATE_PIN(PIOB, 1)
#define CS_2            IOPORT_CREATE_PIN(PIOB, 2)
#define CS_3            IOPORT_CREATE_PIN(PIOB, 3)
#define CS_4            IOPORT_CREATE_PIN(PIOA, 17)
#define CS_5            IOPORT_CREATE_PIN(PIOA, 18)
#define CS_6            IOPORT_CREATE_PIN(PIOA, 19)
#define CS_OSC_ATTEN    IOPORT_CREATE_PIN(PIOA, 10)   // also known as "CS7"
#define CS_SWEEP_ATTEN  IOPORT_CREATE_PIN(PIOA, 6)    // also known as "CS8"

//#define CS_SW           IOPORT_CREATE_PIN(PIOA, 18)

#define MOSI            IOPORT_CREATE_PIN(PIOA, 13)
#define SCK             IOPORT_CREATE_PIN(PIOA, 14)

//#define SWITCH_2        IOPORT_CREATE_PIN(PIOB, 1)
//#define SWITCH_3        IOPORT_CREATE_PIN(PIOB, 2)
//#define SWITCH_4        IOPORT_CREATE_PIN(PIOB, 3)

#define DEVICE_SCL      IOPORT_CREATE_PIN(PIOA, 4)
#define DEVICE_SDA      IOPORT_CREATE_PIN(PIOA, 3)
#define DEVICE_TWI      TWI0


#define DRV_1_ADDR  (0xA0 | 0 << 1)
#define DRV_2_ADDR  (0xA0 | 1 << 1)
#define DRV_3_ADDR  (0xA0 | 2 << 1)
#define DRV_4_ADDR  (0xA0 | 3 << 1)


/* =============== TWI1 =============== */
#define PINS_DEVICE_TWI          (PIO_PA4A_TWCK0 | PIO_PA3A_TWD0)
#define PINS_DEVICE_TWI_FLAGS    (PIO_PERIPH_A | PIO_DEFAULT)
#define PINS_DEVICE_TWI_MASK     (PIO_PA4A_TWCK0 | PIO_PA3A_TWD0)
#define PINS_DEVICE_TWI_PIO      PIOA
#define PINS_DEVICE_TWI_ID       ID_PIOA
#define PINS_DEVICE_TWI_TYPE     PIO_PERIPH_A
#define PINS_DEVICE_TWI_ATTR     PIO_DEFAULT


/* =============== Debug UART =============== */
#define DEBUG_UART                 (Usart*)UART3
#define DEBUG_UART_ID              ID_UART3
#define DEBUG_UART_BAUDRATE        115200
#define DEBUG_UART_MODE            UART_MR_PAR_NO
#define DEBUG_UART_CHAR_LENGTH     8
#define DEBUG_UART_PARITY          UART_MR_PAR_NO
#define DEBUG_UART_STOP_BIT        false

#define DEBUG_UART_PIN_TX         IOPORT_CREATE_PIN(PIOB, 11)
#define DEBUG_UART_PIN_RX         IOPORT_CREATE_PIN(PIOB, 10)

#define DEBUG_UART_IRQ            UART3_IRQn
#define DEBUG_UART_ISR_HANDLER    UART3_Handler



#endif // CONF_BOARD_H
