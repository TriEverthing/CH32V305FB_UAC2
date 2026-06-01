/* private includes ----------------------------------------------------------*/
#include "ch32v30x_bsp.h"

/* private typedef -----------------------------------------------------------*/

/* private define ------------------------------------------------------------*/

/* private macro -------------------------------------------------------------*/

/* private variables ---------------------------------------------------------*/
/* at-start led resouce array */
GPIO_TypeDef *led_gpio_port[BSP_LED_NUM]  			= { BSP_LED0_GPIO };
const uint16_t led_gpio_pin[BSP_LED_NUM]       	= { BSP_LED0_NUM  };
const uint32_t led_gpio_crm_clk[BSP_LED_NUM] 		= { BSP_LED0_CLK_EN };
/* exported variables --------------------------------------------------------*/
#define UARTPR                USART1
#define USART_TX_BUF_SIZE     512
#define USART_RX_BUF_SIZE     512
uint8_t uart_is_initialized = 0 ;
#if UART_DMA_TX
uint8_t uart_tx_buf1[USART_TX_BUF_SIZE];
uint8_t uart_tx_buf2[USART_TX_BUF_SIZE];
uint8_t *uart_tx_buf = uart_tx_buf1 ;
uint16_t uart_tx_ptr = 0 ;
uint8_t uart_tx_buf_idle_flag = 1 ;
uint8_t uart_tx_dma_busy_flag = 0 ;
#endif
/* delay variable */
__IO uint8_t  UsNumber = 0 ;
static uint16_t MsNumber = 0 ;
/* tick */
__IO uint32_t overTick = 0 ;
__IO uint32_t uwTick = 0 ;
uint16_t uwTickFreq = 1 ;  /* 1KHz */
/* private function prototypes --------------------------------------------*/

/* private user code ------------------------------------------------------*/

extern uint32_t SystemCoreClock ;  

/**
  * @brief  this function configures the source of the time base.
  * @param  none
  * @retval none
  */
void bsp_timebase_init(void)
{
	UsNumber = SystemCoreClock / 8000000 ;
  MsNumber = (uint16_t)UsNumber * 1000 ;
	/* system tick config */
	TICK_COUNT_VALUE = 0UL ;
	SysTick->CMP = 0x00 ;
  SysTick->CTLR &= ~ ( SysTick_CTRL_MODE_Msk | SysTick_CTRL_STCLK_Msk );
	SysTick->CTLR |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_STRE_Msk ;
}


/**
  * @brief  delay ms function
  * @param  none
  * @retval none
  */
void soft_delay_ms( uint32_t dms )
{
	uint32_t mt1 , mt2 ;
	for( mt1 = 0 ; mt1 < dms ; mt1 ++ )
	{
		for( mt2 = 0 ; mt2 < 28800 ; mt2 ++ ) //1ms
		{
		  __NOP();
		  __NOP();
		}
	}
}

/**
  * @brief  delay us function
  * @param  none
  * @retval none
  */
void soft_delay_us( uint32_t dus )
{
	uint32_t mt1 ;
	uint8_t mt2 ;
	for( mt1 = 0 ; mt1 < dus ; mt1 ++ )
	{
		for( mt2 = 0 ; mt2 < 16 ; mt2 ++ ) //1us
		{
		  __NOP();
		}
	}
}

/**
 * @brief  None.
 * @param  None.
 * @return None.
 */
void DebugInit( void )
{
  uart_print_init( 115200 );
#if Usb_To_Uart
  InitUSBDevice();
#endif

#if USE_EasyLogger
  //easylogger init
  elog_init();
  //Assert
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  //Error
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG);
  //Waring
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG);
  //Information
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
  //Debug
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LINE | ELOG_FMT_FUNC | ELOG_FMT_TIME );
  //Verbose
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);
  //Start elog
  elog_start();
#endif
}

extern uint16_t usb_vcp_write_buf(const char* data , uint16_t length );

uint16_t debug_printf(const char * format , ... )
{
  va_list ap;
  uint16_t len ;
  char debug_buff[256];
  va_start(ap,format);
  len = vsnprintf(debug_buff,256,format,ap);
  va_end(ap);
  uart_write_buf( debug_buff , len );
  usb_vcp_write_buf( debug_buff , len );
	return len ;
}

//Redefine the fputc function
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
PUTCHAR_PROTOTYPE
{
    while( ( USART1->STATR & USART_STATR_TC ) == 0 );
    USART1->DATAR = (uint8_t) ( uint8_t ) ch ;
	return ch;
}


/*******************************************************************************
* Function Name  : uart_init
* Description    : Initialize the I/O serial port 1
* Input          : baud rates
* Return         : none
*******************************************************************************/
void uart_print_init(uint32_t baudrate)
{
    uint32_t  tmpreg = 0x00, pclock ;
    uint32_t  integerdivider = 0x00;
    uint32_t  fractionaldivider = 0x00;
    uart_tx_buf_idle_flag = 1 ;
    uart_tx_dma_busy_flag = 0 ;
    //Enable USART1 Clocks
    RCC->APB2PCENR |= RCC_USART1EN ; 
    //Enable GPIOA 
    RCC->APB2PCENR |= RCC_IOPAEN | RCC_AFIOEN ;
    //PA9 bandwith PA13 SWDIO , switch off SWD
    AFIO->PCFR1 = ( AFIO->PCFR1 & ( ~AFIO_PCFR1_SWJ_CFG ) ) | AFIO_PCFR1_SWJ_CFG_DISABLE ;
    //UART1TX PA9
    GPIOA->CFGHR = ( GPIOA->CFGHR  & ( ~( GPIO_CFGHR_MODE9 | GPIO_CFGHR_CNF9 ) ) ) | GPIO_CFGHR_CNF9_1 | GPIO_CFGHR_MODE9 ;
    
    //USART1 Init
    //8bits , no Parity , OnlyTX enable
    USART1->CTLR1 &= ~( USART_CTLR1_M | USART_CTLR1_PCE | USART_CTLR1_PS | USART_CTLR1_RE | USART_CTLR1_TE );
    USART1->CTLR1 |= USART_CTLR1_TE ;
    //1 StopBits 
    USART1->CTLR2 &= ~USART_CTLR2_STOP ;
    //No HardFlow
    USART1->CTLR3 &= ~( USART_CTLR3_RTSE | USART_CTLR3_CTSE ) ;

    //Baudrate Setting
    //pclik
    pclock = SystemCoreClock ;
    if( ( USART1->CTLR1 & USART_CTLR1_M_EXT6 ) == 0 ) 
        integerdivider = ( 25 * pclock ) / 4  / baudrate ; 
    else
        integerdivider = ( 25 * pclock ) / 2  / baudrate ; 

    tmpreg = (integerdivider / 100) << 4 ;
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4)) ;

    if( ( USART1->CTLR1 & USART_CTLR1_M_EXT6 ) == 0 ) 
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F) ;
    else
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07) ;

    USART1->BRR = (uint16_t)tmpreg ;

#if UART_DMA_TX
    //Enable DMA1 Clokcs
    RCC->AHBPCENR |= RCC_DMA1EN ;
    //TX/RX DMA Enable
	  USART1->CTLR3 |= USART_CTLR3_DMAT ;
    //DMA Channel4 USART1_TX
    //from Memory to Peripheral , Signal mode , Peripheral inc , Peripheral Size = 8bit , Memeory Size = 8bit
    DMA1_Channel4->CFGR = DMA_CFG4_DIR | DMA_CFG4_MINC | DMA_CFG4_PL_1 ;
    //Trans number
    DMA1_Channel4->CNTR = 0U ;
    //Peripheral Address
    DMA1_Channel4->PADDR = ( uint32_t )( &( USART1->DATAR ));
    ////Memory Address
    //DMA1_Channel4->MADDR = ( uint32_t ) uart_tx_buf ;
    //DMA1_Channel4_IRQn Priority
    //bit[7] - Preemption Priority = 1 , bit[6:5] - Sub priority = 3
    NVIC->IPRIOR[(uint32_t)(DMA1_Channel4_IRQn)] = ( 1 << 7 ) | ( 3 << 5 );
    //Enable DMA1 IRQ
    NVIC->IENR[((uint32_t)( DMA1_Channel4_IRQn ) >> 5)] = (1 << ((uint32_t)(DMA1_Channel4_IRQn) & 0x1F));
    //Enable DMA1 Channel2 for USART1_TX , full or half interrupt
    DMA1_Channel4->CFGR |= DMA_CFG4_TCIE ;
    //Channel Eenbale
    //DMA1_Channel2->CFGR |= DMA_CFGR2_EN ;
    uart_tx_ptr = 0 ;
#endif

    //USART1 Enable
    USART1->CTLR1 |= USART_CTLR1_UE ;
    uart_is_initialized = 1 ;
}

#if UART_DMA_TX
void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void)
{
	if( DMA1->INTFR & DMA_TCIF4 )
	{
    if( uart_tx_ptr > 0 && uart_tx_buf_idle_flag )
    {
      DMA1_Channel4->CFGR &= ~DMA_CFG4_EN ;
      //Trans number
      DMA1_Channel4->CNTR = uart_tx_ptr ;
      //Memory Address
      DMA1_Channel4->MADDR = ( uint32_t )uart_tx_buf ;
      //enbale DMA
      DMA1_Channel4->CFGR |= DMA_CFG4_EN ;
      uart_tx_ptr = 0 ;

      if( uart_tx_buf == uart_tx_buf1 )
        uart_tx_buf = uart_tx_buf2 ;
      else 
        uart_tx_buf = uart_tx_buf1 ;
    }
    else
    {
        uart_tx_dma_busy_flag = 0 ;
    }
    //Clear Flag
	  DMA1->INTFCR |= DMA_CTCIF4 ;
	}
}
#endif

#if UART_DMA_TX  
void uart_refresh_buf(void)
{
  if( uart_is_initialized == 0 )
    return;

  if( uart_tx_ptr > 0 && uart_tx_dma_busy_flag == 0 )
  {
    uart_tx_dma_busy_flag = 1 ;
    DMA1_Channel2->CFGR &= ~DMA_CFGR2_EN ;
    //Trans number
    DMA1_Channel2->CNTR = uart_tx_ptr ;
    //Memory Address
    DMA1_Channel2->MADDR = ( uint32_t ) uart_tx_buf ;
    //enbale DMA
    DMA1_Channel2->CFGR |= DMA_CFGR2_EN ;
    uart_tx_ptr = 0 ;

    if( uart_tx_buf == uart_tx_buf1 )
      uart_tx_buf = uart_tx_buf2 ;
    else 
      uart_tx_buf = uart_tx_buf1 ;
  }
}
#endif
/*******************************************************************************
* Function Name  : usart_printf
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint16_t uart_printf(const char* fmt,...)
{
	uint16_t len_t ;
	va_list ap ;

#if UART_DMA_TX  
  uart_tx_buf_idle_flag = 0 ;
  va_start(ap,fmt);
	len_t = vsnprintf((char*)uart_tx_buf+uart_tx_ptr,USART_TX_BUF_SIZE,fmt,ap);
	va_end(ap);
  uart_tx_ptr = len_t + 1 ;
  uart_refresh_buf();
  uart_tx_buf_idle_flag = 1 ;
#else
	uint8_t uart_tx_buf[USART_TX_BUF_SIZE];

  if( uart_is_initialized == 0 )
    return 0 ;

  va_start(ap,fmt);
	len_t = vsnprintf((char*)uart_tx_buf,USART_TX_BUF_SIZE,fmt,ap);
	va_end(ap);
	len_t = len_t + 1 ;
  for( uint16_t i = 0 ; i < len_t ; i++ )
	{
		while( ( USART1->STATR & USART_STATR_TC ) == 0 );
		USART1->DATAR = (uint8_t) uart_tx_buf[i];
	}
#endif
  return len_t;
}

/*******************************************************************************
* Function Name  : uart_write_buf
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint16_t uart_write_buf(const char* data , uint16_t length )
{
  uint16_t len_t ;
#if UART_DMA_TX
  uart_tx_buf_idle_flag = 0 ;
  len_t = USART_TX_BUF_SIZE - uart_tx_ptr ;
  len_t = length > len_t ? len_t : length ;
  memcpy( uart_tx_buf + uart_tx_ptr , data , len_t );
  uart_tx_ptr = uart_tx_ptr + len_t ;
  uart_refresh_buf();
  uart_tx_buf_idle_flag = 1 ;
#else
	uint8_t uart_tx_buf[USART_TX_BUF_SIZE];

  if( uart_is_initialized == 0 )
    return 0 ;

  len_t = length > USART_TX_BUF_SIZE ? USART_TX_BUF_SIZE : length ;
  for( uint16_t i = 0 ; i < len_t ; i++ )
  {
		while( ( USART1->STATR & USART_STATR_TC ) == 0 );
		USART1->DATAR = (uint8_t) uart_tx_buf[i];
  }
#endif
  return len_t;
}

/**
  * @brief  configure led gpio
  * @param  led: specifies the led to be configured.
  * @retval none
  */
void led_init(led_type led)
{
    uint32_t Shift_Mask  = 0x00 ;
	/* enable the led clock */
    RCC->APB2PCENR |= led_gpio_crm_clk[led] ;

    if( led_gpio_pin[led] < 8 )
    {
        Shift_Mask = led_gpio_pin[led] << 2 ;
	    /* configure the led gpio */
        led_gpio_port[led]->CFGLR &= ~( ((uint32_t)0x0000000F) << Shift_Mask ) ;
        led_gpio_port[led]->CFGLR |=  ( ((uint32_t)0x00000003) << Shift_Mask ) ;
    }
    else 
    {
        Shift_Mask = ( led_gpio_pin[led] - 8 ) << 2 ;
    	/* configure the led gpio */
        led_gpio_port[led]->CFGHR &= ~( ((uint32_t)0x0000000F) << Shift_Mask ) ;
        led_gpio_port[led]->CFGHR |=  ( ((uint32_t)0x00000003) << Shift_Mask ) ;
    } 
    led_on( led );
}

/**
  * @brief  turns selected led on.
  * @param  led: specifies the led to be set on.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval none
  */
void led_on(led_type led)
{
  if(led > (BSP_LED_NUM - 1))
    return;
  if(led_gpio_pin[led])
    led_gpio_port[led]->BSHR = (( uint32_t)0x00000001) << led_gpio_pin[led] ;

}

/**
  * @brief  turns selected led off.
  * @param  led: specifies the led to be set off.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval none
  */
void led_off(led_type led)
{
  if(led > (BSP_LED_NUM - 1))
    return;
  if(led_gpio_pin[led])
    led_gpio_port[led]->BCR = (( uint32_t)0x00000001) << led_gpio_pin[led] ;
}

/**
  * @brief  turns selected led toggle.
  * @param  led: specifies the led to be set off.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval none
  */
void led_toggle(led_type led)
{
	if( led > ( BSP_LED_NUM - 1 ) )
		return;
	if(led_gpio_pin[led])
		led_gpio_port[led]->OUTDR ^= (( uint32_t)0x00000001) << led_gpio_pin[led]  ;
}

/**
  * @brief  configure button gpio
  * @param  button: specifies the button to be configured.
  * @retval none
  */
void bsp_button_init(void)
{
  /* enable the button clock */
  //Enable GPIOC
  RCC->APB2PCENR |= BSP_Button0_CLK_EN ;

  /* configure button pin as input with pull-up/pull-down */
  //Button0 PC2
#if BSP_Button0_NUM < 8
  BSP_Button0_GPIO->CFGLR &= ~( ((uint32_t)0x0000000F) << ( BSP_Button0_NUM << 2 ) ) ;
  BSP_Button0_GPIO->CFGLR |=  ( ((uint32_t)0x00000008) << ( BSP_Button0_NUM << 2 ) ) ;
#else
  BSP_Button0_GPIO->CFGHR &= ~( ((uint32_t)0x0000000F) << ( ( BSP_Button0_NUM - 8 ) << 2 ) ) ;
  BSP_Button0_GPIO->CFGHR |=  ( ((uint32_t)0x00000008) << ( ( BSP_Button0_NUM - 8 ) << 2 ) ) ;
#endif
  BSP_Button0_GPIO->OUTDR |= ( ( uint16_t )( 0x0001 ) << BSP_Button0_NUM ) ;
}

/**
  * @brief  returns the selected button state
  * @param  none
  * @retval the button gpio pin value
  */
uint8_t bsp_button_state(void)
{
  return ( ( uint8_t )( BSP_Button0_GPIO->INDR >> BSP_Button0_NUM ) & 0x01 ) ;
}

/**
  * @brief  returns which button have press down
  * @param  none
  * @retval the button have press down
  */
button_type bsp_button_press( void )
{
  static uint8_t pressed = 1;
  /* get button state in at_start board */
  if((pressed == 1) && (bsp_button_state() == RESET))
  {
    /* debounce */
    pressed = 0;
    soft_delay_ms(10);
    if(bsp_button_state() == RESET )
      return PRESS_BUTTON;
  }
  else if(bsp_button_state() != RESET)
  {
    pressed = 1;
  }
  return NO_BUTTON;
}

/**
  * @brief  initialize delay function
  * @param  none
  * @retval none
  */
void delay_init( void )
{
  UsNumber = SystemCoreClock / 8000000 ;
  MsNumber = (uint16_t)UsNumber * 1000 ;
  SysTick->CTLR &= ~ ( SysTick_CTRL_MODE_Msk | SysTick_CTRL_STCLK_Msk );
  SysTick->CTLR |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_STRE_Msk ;
}

/**
  * @brief  inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval none
  */
void delay_us(uint32_t nus)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0) ;
    i = (uint32_t)nus * UsNumber ;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while((SysTick->SR & (1 << 0)) != (1 << 0));
    SysTick->CTLR &= ~(1 << 0) ;
}
/**
  * @brief  inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval none
  */
void delay_ms(uint32_t nms)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)nms * MsNumber ;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while((SysTick->SR & (1 << 0)) != (1 << 0));
    SysTick->CTLR &= ~(1 << 0);
}



/**
  * @brief  inserts a delay time.Will not cause task scheduling
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval none
  */
void delay_xms(uint32_t nms)
{
	uint32_t i;
	for( i = 0 ; i < nms ; i ++ )
	{
		delay_us( 1000 ) ;
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

