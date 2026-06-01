#include "ch32v30x.h"
#include "usbd_core.h"
#include "stdarg.h"
#include "bootuf2.h"

#define USBHS_DEV_PU_EN         (1<<4)
#define USBHS_ALL_CLR           (1<<1)
#define USBHS_FORCE_RST         (1<<2)


void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SW_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void uart_printf_init(uint32_t baudrate);
void uart_printf(const char* fmt,...);
void Jump_To_Application(void);
uint8_t BootButtom_Check(void);
void soft_delay_ms( uint32_t dms , uint8_t Sysclock );
void SetSysClockTo96_HSE(void);
void SetSysClockTo96_HSI(void);

extern void msc_bootuf2_init(uint8_t busid, uintptr_t reg_base);

int main( void )
{
    uint32_t main_ticks = 0 ;

    //Startup HSI 8MHz, Sysclock = 8MHz
    /* Check if the Key push-button */
    if( BootButtom_Check() == 0x01 )
    {
        /* Test if user code is programmed starting from address  */
        if( (*(__IO uint32_t*)(FIRMWARE_START_ADDR) ) != 0xE339E339 )
        {
            Jump_To_Application();
            while (1); 
        }
    }

    /* Otherwise enters DFU mode to allow user to program his application */
    SetSysClockTo96_HSI();

#if DEBUG_EN
    //usart init
    uart_printf_init( 115200 );

    uart_printf("Enter into Bootloader.\r\n");
#endif 
    //led init
    //Enable GPIOC Clocks
    RCC->APB2PCENR |= RCC_IOPCEN ;
    //LED1 PC8 configed as output , speed 50MHz
    GPIOC->CFGHR = ( GPIOC->CFGHR  & ( ~( GPIO_CFGHR_MODE8 | GPIO_CFGHR_CNF8 ) ) ) | GPIO_CFGHR_MODE8 ;
    //GPIO output high
    GPIOC->OUTDR |= GPIO_OUTDR_ODR8 ;

    //CherryUSB device msc init
    msc_bootuf2_init( 0 , USBFS_BASE );
    // Wait until configured
    while( usb_device_is_configured( 0 ) == 0 )
    {
        if( main_ticks % 1000 == 0 )
        {
            GPIOC->OUTDR ^= GPIO_OUTDR_ODR8 ;
        }
        
        soft_delay_ms( 1 , 96 );   
        main_ticks ++ ;         
    };

    while(1)
    {
        if ( bootuf2_is_write_done() )
        {
            Jump_To_Application();
            while(1) ;
        }

        if( main_ticks % 100 == 0 )
        {
            GPIOC->OUTDR ^= GPIO_OUTDR_ODR8 ;
        }
        
        soft_delay_ms( 1 , 96 );   
        main_ticks ++ ; 
    }
    return 0;
}

/*
 * @fn      cherryusb usb_dc_low_level_init
 *
 * @brief   Initializes the clock for USB2.0 High speed device.
 *
 * @return  none
 */
void usb_dc_low_level_init(void)
{
    //Enbale AFIO
    RCC->APB2PCENR |= RCC_AFIOEN | RCC_IOPBEN ;
#ifdef  CONFIG_USB_HS
	//RCC->CFGR2[31] : USB clock 48MHz source 
    //0 -- PLLCLK clock selected as USB clock 48MHz clock entry
    //1 -- USBPHY clock selected as USB clock 48MHz clock entry 
    RCC->CFGR2 = ( RCC->CFGR2 & ( ~ RCC_USBFSSRC ) ) | ( 1 << 31 ); 

    /* USB PLL Setting */
    //RCC->CFGR2[27] : USBHSPLL clock source
    //0 -- HSE clock selected as USBHSPLL clock entry
    //1 -- HSI clock selected as USBHSPLL clock entry
    RCC->CFGR2 = ( RCC->CFGR2 & ( ~ RCC_USBHSPLLSRC ) ) | ( 0 << 27 ); 
    
    //RCC->CFGR2[26:24] : USBHS clock divider
    //USBHS clock = USBPLL / ( RCC->CFGR2[26:24] + 1 )
    //RCC->CFGR2[29:28] : Configures the USBHSPLL reference clock
    //USBHSPLL reference clock = USBHSPLLSRC / ( RCC->CFGR2[26:24] + 1 )
    //0 -- reference clock 3MHz.
    //1 -- reference clock 4MHz.
    //2 -- reference clock 8MHz.
    //3 -- reference clock 5MHz.
    RCC->CFGR2 &= ~( RCC_USBHSDIV | RCC_USBHSCLK ) ;
#if HSE_VALUE == 8000000
    RCC->CFGR2 |= ( 1 << 24 ) | ( 1 << 28 ) ;
#elif HSE_VALUE == 16000000
    RCC->CFGR2 |= ( 3 << 24 ) | ( 1 << 28 ) ;
#elif HSE_VALUE == 20000000
    RCC->CFGR2 |= ( 3 << 24 ) | ( 3 << 28 ) ;
#elif HSE_VALUE == 24000000
    RCC->CFGR2 |= ( 5 << 24 ) | ( 1 << 28 ) ;
#else
    #error "Unsupported HSE frequency!"
#endif

    //Enable USBHS PHY
    RCC->CFGR2 |= RCC_USBHSPLL ;
	//Enables or disables the AHB peripheral clock
    RCC->AHBPCENR |= RCC_USBHSEN ;

	//NVIC_IRQChannelPreemptionPriority - range from 0 to 3.
	//NVIC_IRQChannelSubPriority - range from 0 to 1.
	NVIC->IPRIOR[(uint32_t)(USBHS_IRQn)] = ( 0 << 6 ) | ( 0 << 5 ) ;
    //Enable USBHS IRQ
    PFIC->IENR[ USBHS_IRQn / 32 ] = 1 << ( USBHS_IRQn & 31 );
#else
    //RCC->CFGR2[31] : USB clock 48MHz source 
    //0 -- PLLCLK clock selected as USB clock 48MHz clock entry
    //1 -- USBPHY clock selected as USB clock 48MHz clock entry 
    RCC->CFGR2 = ( RCC->CFGR2 & ( ~ RCC_USBFSSRC ) ) | ( 1 << 31 ); 

    /* USB PLL Setting */
    //RCC->CFGR2[27] : USBHSPLL clock source
    //0 -- HSE clock selected as USBHSPLL clock entry
    //1 -- HSI clock selected as USBHSPLL clock entry
    RCC->CFGR2 = ( RCC->CFGR2 & ( ~ RCC_USBHSPLLSRC ) ) | ( 1 << 27 ); 

    //RCC->CFGR2[26:24] : USBHS clock divider
    //USBHS clock = USBPLL / ( RCC->CFGR2[26:24] + 1 )
    //RCC->CFGR2[29:28] : Configures the USBHSPLL reference clock
    //USBHSPLL reference clock = USBHSPLLSRC / ( RCC->CFGR2[26:24] + 1 )
    //0 -- reference clock 3MHz.
    //1 -- reference clock 4MHz.
    //2 -- reference clock 8MHz.
    //3 -- reference clock 5MHz.
    RCC->CFGR2 = ( RCC->CFGR2 & ( ~( RCC_USBHSDIV | RCC_USBHSCLK ))) | ( 1 << 24 ) | ( 1 << 28 ) ;

    //Enable USBHS PHY
    RCC->CFGR2 |= RCC_USBHSPLL;
	//Enables the AHB peripheral clock
    RCC->AHBPCENR |= RCC_OTGFSEN | RCC_USBHSEN ;
	//NVIC_IRQChannelPreemptionPriority - range from 0 to 3.
	//NVIC_IRQChannelSubPriority - range from 0 to 1.
	NVIC->IPRIOR[(uint32_t)(USBHS_IRQn)] = ( 0 << 6 ) | ( 0 << 5 ) ;
    //Enable USBHS IRQ
    PFIC->IENR[ USBHS_IRQn / 32 ] = 1 << ( USBHS_IRQn & 31 );
#endif
}

/*********************************************************************
 * @fn      PC9_Check
 *
 * @brief   Check PC9 state
 *
 * @return  1 - IAP
 *          0 - APP
 */
uint8_t BootButtom_Check(void)
{
    uint8_t i, cnt = 0 ;

    //Enable GPIOC Clocks
    RCC->APB2PCENR |= RCC_IOPCEN ;
    //PC9 configed as input with pull up/down resisters
    GPIOC->CFGHR = ( GPIOC->CFGHR  & ( ~( GPIO_CFGHR_MODE9 | GPIO_CFGHR_CNF9 ) ) ) | GPIO_CFGHR_CNF9_1 ;
    //enbale pullup
    GPIOC->OUTDR |= GPIO_OUTDR_ODR9 ;

    for( i = 0 ; i < 10 ; i++ )
    {
        if( ( GPIOC->INDR & GPIO_INDR_IDR9 ) == 0 ) 
            cnt++;
        soft_delay_ms( 5 , 8 );
    }

    if( cnt > 6 ) 
        return 0;
    else 
        return 1;
}

/**
  * @brief  delay ms function
  * @param  none
  * @retval none
  */
void soft_delay_ms( uint32_t dms , uint8_t Sysclock )
{
	uint32_t mt1 , mt2  = 0 ;
    uint32_t tick_ms = 200 ;
    tick_ms = tick_ms * Sysclock ;
	for( mt1 = 0 ; mt1 < dms ; mt1 ++ )
	{
		for( mt2 = 0 ; mt2 < tick_ms ; mt2 ++ ) //1ms
		{
		  __NOP();
		  __NOP();
		}
	}
}

/*********************************************************************
 * @fn      IAP_2_APP
 *
 * @brief   IAP_2_APP program.
 *
 * @return  none
 */
void Jump_To_Application(void)
{
    //USB HS Disable
    NVIC_DisableIRQ( USBHS_IRQn );
    USBHSD->HOST_CTRL = 0x00 ;
    USBHSD->CONTROL = 0x00 ;
    USBHSD->INT_EN = 0x00 ;
    USBHSD->ENDP_CONFIG = 0xFFFFFFFF ;
    USBHSD->CONTROL &= ~USBHS_DEV_PU_EN ;
    USBHSD->CONTROL |= USBHS_ALL_CLR | USBHS_FORCE_RST ;
    USBHSD->CONTROL = 0x00 ;
    //soft_delay_ms(50);
    //Disable GPIO
    RCC->APB2PRSTR |= RCC_IOPCRST ;
    RCC->APB2PRSTR &= ~RCC_IOPCRST ;
    //Disable GPIOC Clock
    RCC->APB2PCENR &= ~RCC_IOPCEN ;
    //Disbale USB HS Clocks
    RCC->AHBPCENR &= ~RCC_USBHSEN ;
    //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, DISABLE);
    //soft_delay_ms(10);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    NVIC_EnableIRQ(Software_IRQn);
    NVIC_SetPendingIRQ(Software_IRQn);
}

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void) 
{
    while(1);
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void) 
{
    while(1);
}
/*********************************************************************
 * @fn      SW_Handler
 *
 * @brief   This function handles Software exception.
 *
 * @return  none
 */
void SW_Handler(void) 
{
    //__asm("li a6, 0x00004000");
    __asm("li a6, %0" : : "i" ( FIRMWARE_START_ADDR ) );
    __asm("jr a6");
    while(1);
}

/*********************************************************************
 * @fn      SetSysClockTo96_HSE
 *
 * @brief   Sets System clock frequency to 96MHz and configure HCLK, PCLK2 and PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo96_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  RCC->CTLR |= (uint32_t)0x00000001;

  RCC->CFGR0 &= (uint32_t)0xF0FF0000;

  RCC->CTLR &= (uint32_t)0xFEF6FFFF;
  RCC->CTLR &= (uint32_t)0xFFFBFFFF;
  RCC->CFGR0 &= (uint32_t)0xFF00FFFF;

#ifdef CH32V30x_D8C
  RCC->CTLR &= (uint32_t)0xEBFFFFFF;
  RCC->INTR = 0x00FF0000;
  RCC->CFGR2 = 0x00000000;
#else
  RCC->INTR = 0x009F0000;   
#endif   

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CTLR & RCC_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01)
  {
    /* HCLK = SYSCLK */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* PCLK1 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /*  PLL configuration: PLLCLK = HSE * 12 = 96 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

#ifdef CH32V30x_D8
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL8);
#else
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL12_EXTEN);
#endif

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  {
        /*
         * If HSE fails to start-up, the application will have wrong clock
     * configuration. User can add here some code to deal with this error
         */
  }
}

/*********************************************************************
 * @fn      SetSysClockTo144_HSI
 *
 * @brief   Sets System clock frequency to 96MHz and configure HCLK, PCLK2 and PCLK1 prescalers.
 *
 * @return  none
 */
void SetSysClockTo96_HSI(void)
{
    RCC->CTLR |= (uint32_t)0x00000001;

    RCC->CFGR0 &= (uint32_t)0xF0FF0000;

    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFF00FFFF;

#ifdef CH32V30x_D8C
    RCC->CTLR &= (uint32_t)0xEBFFFFFF;
    RCC->INTR = 0x00FF0000;
    RCC->CFGR2 = 0x00000000;
#else
    RCC->INTR = 0x009F0000;   
#endif   

    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* HCLK = SYSCLK */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* PCLK1 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /*  PLL configuration: PLLCLK = HSI * 12 = 96 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

#ifdef CH32V30x_D8
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL12);
#else
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL12_EXTEN);
#endif

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}


/*********************************************************************
 * @fn      Uart_printf_init
 *
 * @brief   Initializes the USARTx peripheral.
 *
 * @param   baudrate - USART communication baud rate.
 *
 * @return  None
 */
void uart_printf_init(uint32_t baudrate)
{
    uint32_t  tmpreg = 0x00, pclock ;
    uint32_t  integerdivider = 0x00;
    uint32_t  fractionaldivider = 0x00;
    //Enable UART1 Clocks
    RCC->APB1PCENR |= RCC_USART1EN ; 
    //Enable GPIOB , AFIO Clocks
    RCC->APB2PCENR |= RCC_IOPBEN | RCC_AFIOEN ;
    //UART3 pin Remap , PB10
    AFIO->PCFR1 &= ~AFIO_PCFR1_USART3_REMAP ;
    //UART3TX2 PB10 init
    GPIOB->CFGHR = ( GPIOB->CFGHR  & ( ~( GPIO_CFGHR_MODE10 | GPIO_CFGHR_CNF10 ) ) ) | GPIO_CFGHR_CNF10_1 | GPIO_CFGHR_MODE10 ;
    
    //UART3 Init
    //8bits , no Parity , OnlyTX enable
    USART3->CTLR1 &= ~( USART_CTLR1_M | USART_CTLR1_PCE | USART_CTLR1_PS | USART_CTLR1_RE | USART_CTLR1_TE );
    USART3->CTLR1 |= USART_CTLR1_TE ;
    //1 StopBits 
    USART3->CTLR2 &= ~USART_CTLR2_STOP ;
    //No HardFlow
    USART3->CTLR3 &= ~( USART_CTLR3_RTSE | USART_CTLR3_CTSE ) ;
    //pclik
    pclock = 96000000 / 2 ; //48MHz
    //Baudrate = FCLK/£¨16*USARTDIV£©= 115200 , USART3's FCLK is PCLK1 , DIV_M[15:4] +£¨ DIV_F[3:0] / 16 £©
    //here FCLK = 48MHz , 16*USARTDIV = 16*DIV_M[15:4] + DIVF[3:0] = 48MHz / Baudrate
    if( ( USART3->CTLR1 & USART_CTLR1_M_EXT6 ) == 0 ) 
        integerdivider = ( 25 * pclock ) / 4  / baudrate ; 
    else
        integerdivider = ( 25 * pclock ) / 2  / baudrate ; 

    tmpreg = (integerdivider / 100) << 4 ;
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4)) ;

    if( ( USART3->CTLR1 & USART_CTLR1_M_EXT6 ) == 0 ) 
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F) ;
    else
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07) ;

    USART3->BRR = (uint16_t)tmpreg ;

    //USART3 Enable
    USART3->CTLR1 |= USART_CTLR1_UE ;
}


/*******************************************************************************
* Function Name  : usart_printf
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void uart_printf(const char* fmt,...)
{
	uint16_t i , len_t ;
	uint8_t uart_printf_buf[256];
	va_list ap;
	va_start(ap,fmt);
	len_t = vsnprintf((char*)uart_printf_buf,sizeof(uart_printf_buf),fmt,ap);
	va_end(ap);
	for( i = 0 ; i < len_t ; i++ )
	{
		while( ( USART3->STATR & USART_STATR_TC ) == 0 );
		USART3->DATAR = (uint8_t) uart_printf_buf[i];
	}
}