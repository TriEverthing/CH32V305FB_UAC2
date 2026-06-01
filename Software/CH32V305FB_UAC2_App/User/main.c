/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#include "ch32v30x_bsp.h"

#include "usbd_core.h"

#include "usb_i2s.h"

#include "acm8625s.h"


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	uint32_t main_tick = 0 ;

	SystemCoreClockUpdate();
	//Debug Init
	DebugInit();
	//led initializing
	led_init(LED0);
	led_off(LED0);

	//usb uac initializing
	usb_i2s_init();

	while( usb_device_is_configured(0) == 0 )
	{
		if( main_tick % 2500 == 0 )
		{
			led_toggle( LED0 );
			dbg_printf("Waiting USB Connecting....\r\n");
		}

		main_tick ++ ;
		soft_delay_ms( 1 );
	}

	acm8625s_init();

	while(1)
    {
		if( main_tick % 500 == 0 )
		{
			led_toggle( LED0 );
			//dbg_printf("CH32 UAC2.0 Speaker:%dHz.\r\n",SystemCoreClock);
		}

		main_tick ++ ;
		soft_delay_ms( 1 );
	}
}
