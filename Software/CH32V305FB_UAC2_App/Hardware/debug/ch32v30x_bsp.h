#ifndef __CH32V30X_BSP_H__
#define __CH32V30X_BSP_H__

/* includes -----------------------------------------------------------------------*/
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
//#include "core_riscv.h"
#include "ch32v30x.h"
#include "system_ch32v30x.h"
/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
typedef enum
{
	LED0                                   = 0,
	LED1                                   = 1,
	LED2                                   = 2
}led_type;

/******************* define button *******************/
typedef enum
{
  PRESS_BUTTON                           = 0,
  NO_BUTTON                              = 1
} button_type;
/* exported macro ------------------------------------------------------------*/
/******************** define led ********************/
#define BSP_LED_NUM                      1
//LED0
#define BSP_LED0_NUM                     8
#define BSP_LED0_GPIO                    GPIOC
#define BSP_LED0_CLK_EN                  RCC_IOPCEN
//Button
#define BSP_Button0_NUM                  9
#define BSP_Button0_GPIO                 GPIOC
#define BSP_Button0_CLK_EN               RCC_IOPCEN

extern void (*Mcu_printf)( const char * format , ... ) ;
/* exported macro ------------------------------------------------------------*/
#define USART_REC_LEN  			200  	//defines the maximum number of received bytes of 200
#define EN_USART1_RX 			  0		  //Enable (1) / Disable (0) Serial port 1 receive
//Receive status flag
extern uint8_t  USART_RX_BUF[USART_REC_LEN];  //Receive buffer, up to USART_REC_LEN bytes. The last byte is '\n'
extern uint16_t USART_RX_STA;
/* exported constants --------------------------------------------------------*/

/* exported macro ------------------------------------------------------------*/
#define DEBUG_ON_OFF			1
#define UART_DMA_TX				1
#define USE_LETTER_SHELL        0
#define USE_EasyLogger          0
#define Usb_To_Uart             0
#define SYSTEM_SUPPORT_OS		1

#if DEBUG_ON_OFF
#if USE_EasyLogger
#define dbg_printf( format , ... )              elog_debug( format , ##__VA_ARGS__ )
#else
#define dbg_printf( format , ... )              debug_printf( format , ##__VA_ARGS__ )
#endif
#else
#define dbg_printf( format , ... )
#endif


//GPIO Bit Band
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//GPIO ODR Addr
#define GPIOA_ODR_Addr    		(GPIOA_BASE+12) //0x4001080C
#define GPIOB_ODR_Addr    		(GPIOB_BASE+12) //0x40010C0C
#define GPIOC_ODR_Addr    		(GPIOC_BASE+12) //0x4001100C
#define GPIOD_ODR_Addr    		(GPIOD_BASE+12) //0x4001140C
#define GPIOE_ODR_Addr    		(GPIOE_BASE+12) //0x4001180C
#define GPIOF_ODR_Addr    		(GPIOF_BASE+12) //0x40011A0C
#define GPIOG_ODR_Addr    		(GPIOG_BASE+12) //0x40011E0C
//GPIO IDR Addr
#define GPIOA_IDR_Addr    		(GPIOA_BASE+8) //0x40010808
#define GPIOB_IDR_Addr    		(GPIOB_BASE+8) //0x40010C08
#define GPIOC_IDR_Addr    		(GPIOC_BASE+8) //0x40011008
#define GPIOD_IDR_Addr    		(GPIOD_BASE+8) //0x40011408
#define GPIOE_IDR_Addr    		(GPIOE_BASE+8) //0x40011808
#define GPIOF_IDR_Addr    		(GPIOF_BASE+8) //0x40011A08
#define GPIOG_IDR_Addr    		(GPIOG_BASE+8) //0x40011E08
//GPIOA
#define PAout(n)   				BIT_ADDR(GPIOA_ODR_Addr,n)
#define PAin(n)    				BIT_ADDR(GPIOA_IDR_Addr,n)
//GPIOB
#define PBout(n)   				BIT_ADDR(GPIOB_ODR_Addr,n)
#define PBin(n)    				BIT_ADDR(GPIOB_IDR_Addr,n)
//GPIOC
#define PCout(n)   				BIT_ADDR(GPIOC_ODR_Addr,n)
#define PCin(n)    				BIT_ADDR(GPIOC_IDR_Addr,n)
//GPIOD
#define PDout(n)   				BIT_ADDR(GPIOD_ODR_Addr,n)
#define PDin(n)    				BIT_ADDR(GPIOD_IDR_Addr,n)
//GPIOE
#define PEout(n)   				BIT_ADDR(GPIOE_ODR_Addr,n)
#define PEin(n)    				BIT_ADDR(GPIOE_IDR_Addr,n)
//GPIOF
#define PFout(n)   				BIT_ADDR(GPIOF_ODR_Addr,n)
#define PFin(n)    				BIT_ADDR(GPIOF_IDR_Addr,n)
//GPIOG
#define PGout(n)   				BIT_ADDR(GPIOG_ODR_Addr,n)
#define PGin(n)    				BIT_ADDR(GPIOG_IDR_Addr,n)
/******************* define time base *****************/
#define SysTick_CTRL_ENABLE_Msk     (uint32_t)(0x00000001)
#define SysTick_CTRL_STIE_Msk       (uint32_t)(0x00000002)
#define SysTick_CTRL_STCLK_Msk      (uint32_t)(0x00000004)
#define SysTick_CTRL_STRE_Msk       (uint32_t)(0x00000008)
#define SysTick_CTRL_MODE_Msk       (uint32_t)(0x00000010)
#define SysTick_CTRL_TICKINT_Msk    (uint32_t)(0x00000020)
#define SysTick_CTRL_SWIE_Msk       (uint32_t)(0x80000000)
#define STEP_DELAY_MS      		    (uint32_t)(50)
#define TICK_COUNT_MAX      	    (uint64_t)(0xFFFFFFFFFFFFFFFF )
#define TICK_COUNT_VALUE      	    (SysTick->CNT)
/* Exported macro ------------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void bsp_timebase_init(void);

void DebugInit( void );

void uart_print_init(uint32_t baudrate);

uint16_t debug_printf(const char * format , ... );

uint16_t uart_printf(const char* fmt,...);

uint16_t uart_write_buf(const char* data, uint16_t length);

void led_init(led_type led);

void led_on(led_type led);

void led_off(led_type led);

void led_toggle(led_type led);

void bsp_button_init(void);

uint8_t bsp_button_state(void);

button_type bsp_button_press( void );

void soft_delay_ms( uint32_t dms );

void soft_delay_us( uint32_t dus );

void delay_init(void);

void delay_ms(uint32_t nms);

void delay_us(uint32_t nus);

void delay_xms(uint32_t nms);

void delay_s(uint32_t ns);

/* External variables --------------------------------------------------------*/


#endif