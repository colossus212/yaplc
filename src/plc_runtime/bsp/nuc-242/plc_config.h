#ifndef _PLC_CONFIG_H_
#define _PLC_CONFIG_H_

/*
*  NUC-227-DEV configuration!
*/

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/cortex.h>

#define PLC_DISABLE_INTERRUPTS cm_disable_interrupts
#define PLC_ENABLE_INTERRUPTS  cm_enable_interrupts

/*
*  PLC clocks
*/
#define PLC_HSE_CONFIG cfg_hse_16Mhz

/*
*  Debug USART
*/
#define DBG_USART        USART3
#define DBG_USART_PERIPH RCC_USART3
#define DBG_USART_VECTOR NVIC_USART3_IRQ
#define DBG_USART_ISR    usart3_isr

#define DBG_USART_TX_PORT GPIOB
#define DBG_USART_RX_PORT GPIOB

#define DBG_USART_TX_PIN GPIO_USART3_TX
#define DBG_USART_RX_PIN GPIO_USART3_RX

#define DBG_USART_TX_PERIPH RCC_GPIOB
#define DBG_USART_RX_PERIPH RCC_GPIOB


/*
*  Jumpers
*/
#define PLC_JMP_RST_PERIPH RCC_GPIOB
#define PLC_JMP_RST_PORT   GPIOB
#define PLC_JMP_RST_PIN    GPIO8

///TODO: Add debug jumper!
//#define PLC_JMP_DBG_PERIPH RCC_GPIOB
//#define PLC_JMP_DBG_PORT   GPIOB
//#define PLC_JMP_DBG_PIN    GPIO9

/*
*  Boot pin
*/
#define PLC_BOOT_PERIPH RCC_GPIOA
#define PLC_BOOT_PORT   GPIOA
#define PLC_BOOT_PIN    GPIO15

/*
*  TODO: Add modbus usart
*/

/*
*  PLC LEDS
*/
#define PLC_LED_STG_PERIPH RCC_GPIOC
#define PLC_LED_STG_PORT   GPIOC
#define PLC_LED_STG_PIN    GPIO6

#define PLC_LED_STR_PERIPH RCC_GPIOC
#define PLC_LED_STR_PORT   GPIOC
#define PLC_LED_STR_PIN    GPIO7

#define PLC_LED_TX_PERIPH RCC_GPIOC
#define PLC_LED_TX_PORT   GPIOC
#define PLC_LED_TX_PIN    GPIO9

#define PLC_LED_RX_PERIPH RCC_GPIOC
#define PLC_LED_RX_PORT   GPIOC
#define PLC_LED_RX_PIN    GPIO8

/*
* PLC Inputs
*/
#define PLC_I1_PERIPH RCC_GPIOB
#define PLC_I1_PORT   GPIOB
#define PLC_I1_PIN    GPIO3

#define PLC_I2_PERIPH RCC_GPIOB
#define PLC_I2_PORT   GPIOB
#define PLC_I2_PIN    GPIO4

#define PLC_I3_PERIPH RCC_GPIOB
#define PLC_I3_PORT   GPIOB
#define PLC_I3_PIN    GPIO6

#define PLC_I4_PERIPH RCC_GPIOB
#define PLC_I4_PORT   GPIOB
#define PLC_I4_PIN    GPIO7

#define PLC_I5_PERIPH RCC_GPIOB
#define PLC_I5_PORT   GPIOB
#define PLC_I5_PIN    GPIO12

#define PLC_I6_PERIPH RCC_GPIOB
#define PLC_I6_PORT   GPIOB
#define PLC_I6_PIN    GPIO13

#define PLC_I7_PERIPH RCC_GPIOB
#define PLC_I7_PORT   GPIOB
#define PLC_I7_PIN    GPIO14

#define PLC_I8_PERIPH RCC_GPIOB
#define PLC_I8_PORT   GPIOB
#define PLC_I8_PIN    GPIO15

/*
* PLC Outputs
*/
#define PLC_O1_PERIPH RCC_GPIOC
#define PLC_O1_PORT   GPIOC
#define PLC_O1_PIN    GPIO10

#define PLC_O2_PERIPH RCC_GPIOC
#define PLC_O2_PORT   GPIOC
#define PLC_O2_PIN    GPIO11

#define PLC_O3_PERIPH RCC_GPIOC
#define PLC_O3_PORT   GPIOC
#define PLC_O3_PIN    GPIO12

#define PLC_O4_PERIPH RCC_GPIOD
#define PLC_O4_PORT   GPIOD
#define PLC_O4_PIN    GPIO2

/*
*  PLC wait timer
*/
#define PLC_WAIT_TMR_PERIPH RCC_TIM7
#define PLC_WAIT_TMR        TIM7
#define PLC_WAIT_TMR_VECTOR NVIC_TIM7_IRQ
#define PLC_WAIT_TMR_ISR    tim7_isr

/*
*  Backup domain offsets
*/
///TODO: correct these!
#define PLC_BKP_VER1_OFFSET      0x0
#define PLC_BKP_VER2_OFFSET      0x4
#define PLC_BKP_RTC_IS_OK_OFFSET 0x8

#define PLC_BKP_REG_OFFSET       0x40
#define PLC_BKP_REG_NUM 32
/*
*  PLC app abi
*/
#define PLC_APP ((plc_app_abi_t *)0x08008000)

/*
*  PLC RTE Version
*/
#define PLC_RTE_VER_MAJOR 3
#define PLC_RTE_VER_MINOR 0
#define PLC_RTE_VER_PATCH 0

/*
*  Logging
*/
#define LOG_LEVELS 4
#define LOG_CRITICAL 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3

/**
* TODO: Add simple printf for error logging!!!
*/
extern const const char plc_iom_err_proto[];
extern const uint32_t plc_iom_err_psz;
#define PLC_IOM_ASSERT(...) plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_iom_err_proto, plc_iom_err_psz)

#endif /* _PLC_CONFIG_H_ */
