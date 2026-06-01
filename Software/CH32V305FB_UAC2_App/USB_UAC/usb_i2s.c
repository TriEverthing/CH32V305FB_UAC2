#include "usb_i2s.h"
#include "usb_config.h"
#include "ch32v30x.h"

#if SAMPLE_BITS == 16
#define DMA_BUFFER_MEMBER_SIZE          4
#else
#define DMA_BUFFER_MEMBER_SIZE          8
#endif

#if AUDIO_OUT_MAX_FREQ <= 64000
#define I2S_DMA_BUFFER_SIZE             512 
#define UAC_BUFFER_LEN                  256
#elif AUDIO_OUT_MAX_FREQ <= 128000
#define I2S_DMA_BUFFER_SIZE             1024
#define UAC_BUFFER_LEN                  512
#elif AUDIO_OUT_MAX_FREQ <= 384000
#define I2S_DMA_BUFFER_SIZE             2048
#define UAC_BUFFER_LEN                  1024
#else 
#error "Not Support audio frequency."
#endif
#define I2S_DMA_BUFFER_SIZE_MASK        (I2S_DMA_BUFFER_SIZE - 1)
// #define UAC_BUFFER_LEN_MASK             (UAC_BUFFER_LEN - 1)
// #define UAC_WPOS_INIT                   (UAC_BUFFER_LEN / 2)

#define FEEDBACK_REPORT_PERIOD          8
#define DMA_FREQUENCY_MEAURE_PERIOD     10

//i2s dma memory buffer
static StereoAudio_t i2s_dma_buffer[I2S_DMA_BUFFER_SIZE] = {0};
static int32_t last_dma_wpos = 0;
//usb uac buffer , when dma buffer can't write , use this buffer
// static StereoAudio_t uac_buffer[UAC_BUFFER_LEN] = {0};
// static uint32_t uac_buf_wpos = UAC_WPOS_INIT;
// static uint32_t uac_buf_rpos = 0;
//
static uint32_t last_dma_can_write ;
//
static uint32_t num_usb = 0;
static uint32_t uac_rx_flag = 0;
static uint32_t sample_rate = 48000 ;
static uint32_t feedback_rate_m = 48000000 ; //mHz

static void wch_i2s_init(void);
static void DMA_Tx_Init(uint32_t memadr, uint32_t bufsize);

/*
 * @fn      cherryusb usb_dc_low_level_init
 *
 * @brief   Initializes the clock for USB2.0 High speed device.
 *
 * @return  none
 */
void usb_dc_low_level_init(void)
{
    RCC->APB2PCENR |= RCC_AFIOEN ;
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
    RCC->CFGR2 &= ~( RCC_USBHSDIV | RCC_USBHSCLK ) ;
    RCC->CFGR2 |= ( 1 << 24 ) | ( 1 << 28 ) ;

    //Enable USBHS PHY
    RCC->CFGR2 |= (1 << 30);
	//Enables or disables the AHB peripheral clock
    RCC->AHBPCENR |= RCC_OTGFSEN ;

	//NVIC_IRQChannelPreemptionPriority - range from 0 to 3.
	//NVIC_IRQChannelSubPriority - range from 0 to 1.
	NVIC->IPRIOR[(uint32_t)(USBFS_IRQ)] = ( 0 << 6 ) | ( 0 << 5 ) ;
    //Enable USBHS IRQ
    PFIC->IENR[ USBFS_IRQ / 32 ] = 1 << ( USBFS_IRQ & 31 );
#endif
}

extern void audio_v2_init(uint8_t busid, uintptr_t reg_base);

void usb_i2s_init(void)
{
    //wch i2s init
    wch_i2s_init();
    //usb uac init
    audio_v2_init( 0 , USBHS_BASE );
}

static void wch_i2s_init(void)
{
    //Enables or disables the APB1 I2S2 peripheral clock
    RCC->APB1PCENR |= RCC_SPI2EN ;
    //Enables or disables the APB2 GPIO peripheral clock
    RCC->APB2PCENR |= RCC_IOPBEN | RCC_IOPCEN ;

    //GPIO peripheral initializes
    //PB12->I2S_WS
    GPIOB->CFGHR = ( GPIOB->CFGHR  & ( ~( GPIO_CFGHR_MODE12 | GPIO_CFGHR_CNF12 ) ) ) | GPIO_CFGHR_CNF12_1 | GPIO_CFGHR_MODE12 ;
    //PB13->I2S_SCK
    GPIOB->CFGHR = ( GPIOB->CFGHR  & ( ~( GPIO_CFGHR_MODE13 | GPIO_CFGHR_CNF13 ) ) ) | GPIO_CFGHR_CNF13_1 | GPIO_CFGHR_MODE13 ;
    //PB15->I2S_DATA
    GPIOB->CFGHR = ( GPIOB->CFGHR  & ( ~( GPIO_CFGHR_MODE15 | GPIO_CFGHR_CNF15 ) ) ) | GPIO_CFGHR_CNF15_1 | GPIO_CFGHR_MODE15 ;
#if ENBALE_I2S_MSCK
    //PC6->I2S_MCK
    GPIOC->CFGLR = ( GPIOC->CFGLR  & ( ~( GPIO_CFGLR_MODE6 | GPIO_CFGLR_CNF6 ) ) ) | GPIO_CFGLR_CNF6_1 | GPIO_CFGLR_MODE6 ;
#endif

    //I2S peripheral initializes
    //clear i2s register bits
    SPI2->I2SCFGR &= 0xF040 ;
    //I2SCFGR[11]: 0-spi mode , 1-i2s mode
    SPI2->I2SCFGR |= 1 << 11 ;
    //I2SCFGR[9:8]:I2S configuration mode , 0-Slave TX , 1-Slave RX , 2-Master TX , 3-Master RX
    SPI2->I2SCFGR |= 2 << 8 ;
    //I2SCFGR[5:4]:I2S standard selection , 0-Phillips , 1-MSB , 2-LSB , 3-PCM
    SPI2->I2SCFGR |= 0 << 4 ;
    //I2SCFGR[3]:steady state clock polarity , 0-low level , 1-high level
    SPI2->I2SCFGR |= 1 << 3 ;
#if SAMPLE_BITS == 16
    //I2SCFGR[2:1]:Data length to be transferred , 0-16bits , 1-24bits , 2-32bits
    SPI2->I2SCFGR |= 0 << 1 ;
    //I2SCFGR[0]:CHLEN : 0-16bits , 1-32-bits
    SPI2->I2SCFGR |= 0 << 0 ;
#elif SAMPLE_BITS == 24
    //I2SCFGR[2:1]:Data length to be transferred , 0-16bits , 1-24bits , 2-32bits
    SPI2->I2SCFGR |= 1 << 1 ;
    //I2SCFGR[0]:CHLEN : 0-16bits , 1-32-bits
    SPI2->I2SCFGR |= 1 << 0 ;
#elif SAMPLE_BITS == 32
    //I2SCFGR[2:1]:Data length to be transferred , 0-16bits , 1-24bits , 2-32bits
    SPI2->I2SCFGR |= 2 << 1 ;
    //I2SCFGR[0]:CHLEN : 0-16bits , 1-32-bits
    SPI2->I2SCFGR |= 1 << 0 ;
#endif
    //clear i2s register bits
    SPI2->I2SPR = 0x0002 ;
#if ENBALE_I2S_MSCK
    //enable i2s mstser clocks outputs 
    SPI2->I2SPR |= SPI_I2SPR_MCKOE ;
#endif
    //setting i2s clocks
    ch32v305_set_i2s_freq(sample_rate);
    //Enables I2S2 DMA interface 
    SPI2->CTLR2 |= SPI_CTLR2_TXDMAEN ;
    //Enables I2S peripheral 
    SPI2->I2SCFGR |= SPI_I2SCFGR_I2SE ;

    DMA_Tx_Init((uint32_t)i2s_dma_buffer, I2S_DMA_BUFFER_SIZE * ( DMA_BUFFER_MEMBER_SIZE / 2 ) );

    //Clears the DMA Channelx's interrupt pending bits.
    DMA1->INTFCR = DMA_CTCIF5 ;
}

static void DMA_Tx_Init(uint32_t memadr, uint32_t bufsize)
{
    dbg_printf("memadr=0x%08X,bufsize=%d.\r\n",memadr,bufsize);
    //Enables the AHB DMA1 peripheral clock
    RCC->AHBPCENR |= RCC_DMA1EN ;

    //DMA peripheral addr
    DMA1_Channel5->PADDR = (uint32_t)&SPI2->DATAR ;
    //DMA memory addr
    DMA1_Channel5->MADDR = memadr ;
    //bufsize
    DMA1_Channel5->CNTR = bufsize ;
    //Diable Mem2Mem , DMA priority very high , Memory DataSize 16bits , Peripheral DataSize 16bits
    //Memory inc enable , Peripheral inc disbale , Circle Mode , Memory to Peripheral
    DMA1_Channel5->CFGR = DMA_CFG5_PL | DMA_CFG5_MSIZE_0 | DMA_CFG5_PSIZE_0 | DMA_CFG5_MINC | DMA_CFG5_CIRC | DMA_CFG5_DIR ;

    //DMA interrupt
    //NVIC_IRQChannelPreemptionPriority - range from 0 to 3.
	//NVIC_IRQChannelSubPriority - range from 0 to 1.
	NVIC->IPRIOR[(uint32_t)(DMA1_Channel5_IRQn)] = ( 0 << 6 ) | ( 2 << 5 ) ;
    //Enable USBHS IRQ
    // PFIC->IENR[ DMA1_Channel5_IRQn / 32 ] = 1 << ( DMA1_Channel5_IRQn & 31 );

    // Enables the specified DMAy Channelx interrupts.
    //DMA1_Channel5->CFGR |= DMA_CFG5_TCIE ;
}

void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void) 
{
    DMA1->INTFCR = DMA_CTCIF5 ;
    // if( uac_rx_flag == 2 )
    // {
    // }
}

void ch32_i2s_start(void) 
{
    dbg_printf("USB_UAC Start.\r\n");
    uac_rx_flag = 1 ;         
    last_dma_wpos = I2S_DMA_BUFFER_SIZE / 2 ;
    last_dma_can_write = last_dma_wpos ;
}

void ch32_i2s_stop(void) 
{
    dbg_printf("USB_UAC Stop.\r\n");
    uac_rx_flag = 2 ;
    num_usb = 0 ;
    DMA1_Channel5->CFGR &= ~DMA_CFG5_EN;
    DMA1_Channel5->CNTR = I2S_DMA_BUFFER_SIZE * ( DMA_BUFFER_MEMBER_SIZE / 2 ) ;
    memset(i2s_dma_buffer, 0, sizeof(i2s_dma_buffer));
}

void ch32_i2s_write(uint8_t* ptr, uint32_t len)  //about 164us for this function , max times = 10us , period = 2 ^( EP_INTERVAL - 4 ) ms = 250us
{
    uint32_t usb_to_dma , cur_dma_can_write , cur_dma_rpos ;
    int32_t delta_dma_can_write ;
    static uint32_t usb_dma_counter ;

#if SAMPLE_BITS == 16
    uint16_t* audio_ptr = (uint16_t*)ptr;
#elif SAMPLE_BITS == 24
    uint16_t* audio_ptr = (uint16_t*)ptr;
#else
    uint32_t* audio_ptr = (uint32_t*)ptr;
#endif

    if( uac_rx_flag == 1 )
    {
        uac_rx_flag = 0 ;
        usb_dma_counter = 0 ;
        //DMA channel enbale
        DMA1_Channel5->CFGR |= DMA_CFG5_EN ;
    }

    cur_dma_rpos = ( I2S_DMA_BUFFER_SIZE * ( DMA_BUFFER_MEMBER_SIZE / 2 )  - DMA1_Channel5->CNTR ) / ( DMA_BUFFER_MEMBER_SIZE / 2 ) ;      
    cur_dma_can_write = ( last_dma_wpos - cur_dma_rpos + I2S_DMA_BUFFER_SIZE ) & I2S_DMA_BUFFER_SIZE_MASK ;

    usb_dma_counter = usb_dma_counter + 1 ;
    delta_dma_can_write = cur_dma_can_write - last_dma_can_write ;
    if( delta_dma_can_write <= -( I2S_DMA_BUFFER_SIZE_MASK - 16 ) || delta_dma_can_write >= ( I2S_DMA_BUFFER_SIZE_MASK - 16 ) )
    {
        last_dma_can_write = cur_dma_can_write ;
        usb_dma_counter = 0 ;
    }
    else if( delta_dma_can_write <= -16 || delta_dma_can_write >= 16 || usb_dma_counter == 40000 )
    {
        delta_dma_can_write = 4000000 * delta_dma_can_write / ((int32_t)usb_dma_counter );
        feedback_rate_m = feedback_rate_m - delta_dma_can_write ;
        last_dma_can_write = cur_dma_can_write ;
        usb_dma_counter = 0 ;
        if( delta_dma_can_write < 0 )
        {
            delta_dma_can_write = -delta_dma_can_write ;
            dbg_printf("data rate=%d.%03dHz,deta=-%d.%03dHz.\r\n",feedback_rate_m/1000,feedback_rate_m%1000,delta_dma_can_write/1000,delta_dma_can_write%1000);
        }
        else
        {
            dbg_printf("data rate=%d.%03dHz,deta=+%d.%03dHz.\r\n",feedback_rate_m/1000,feedback_rate_m%1000,delta_dma_can_write/1000,delta_dma_can_write%1000);
        }

    }

    //copy new usb to dma
    usb_to_dma = len / HALF_WORD_BYTES / OUT_CHANNEL_NUM ;
    num_usb += usb_to_dma ;
#if SAMPLE_BITS == 16
    while (usb_to_dma--) 
    {
        i2s_dma_buffer[last_dma_wpos].left = *audio_ptr;
        audio_ptr = audio_ptr + 1 ;
        i2s_dma_buffer[last_dma_wpos].right = *audio_ptr;
        audio_ptr = audio_ptr + 1 ;
        last_dma_wpos = (last_dma_wpos + 1) & I2S_DMA_BUFFER_SIZE_MASK;
    }
#elif SAMPLE_BITS == 24
    while (usb_to_dma--) 
    {
        i2s_dma_buffer[last_dma_wpos].left = ( audio_ptr[0] << 16 ) | ( audio_ptr[1] & 0x00FF ) ;
        audio_ptr = audio_ptr + 1 ;
        i2s_dma_buffer[last_dma_wpos].right = ( audio_ptr[0] >> 8 ) | ( audio_ptr[1] << 8 ) | ( audio_ptr[1] >> 8 ) ;
        audio_ptr = audio_ptr + 2 ;
        last_dma_wpos = (last_dma_wpos + 1) & I2S_DMA_BUFFER_SIZE_MASK;
    }
#else
    while(usb_to_dma--) 
    {
        i2s_dma_buffer[last_dma_wpos].left  = ((*audio_ptr & 0x0000FFFF) << 16 ) | ((*audio_ptr & 0xFFFF0000) >> 16 );
        audio_ptr = audio_ptr + 1 ;
        i2s_dma_buffer[last_dma_wpos].right = ((*audio_ptr & 0x0000FFFF) << 16 ) | ((*audio_ptr & 0xFFFF0000) >> 16 );
        audio_ptr = audio_ptr + 1 ;
        last_dma_wpos = (last_dma_wpos + 1) & I2S_DMA_BUFFER_SIZE_MASK;
    }
#endif
}

void ch32v305_set_i2s_freq(uint32_t sampling_freq)
{
    uint16_t div = 5 , pll3_mul_reg = 15 ;
    uint8_t i2s_use_pll3 = 0 , pll3_mul = 20 ;

    sample_rate = sampling_freq ;

    if( ( sampling_freq % 8000 ) * 10 < sampling_freq / 1000 ) //error <= 0.01%
    {
        div = 384000 / sampling_freq * 5 ;
        i2s_use_pll3 = 0 ;
    }
    else if( ( 8000 - ( sampling_freq % 8000 ) )*10 < sampling_freq / 1000  ) //error <= 0.01%
    {
        div = ( 384000 / sampling_freq + 1 ) * 5 ;
        i2s_use_pll3 = 0 ;
    }
    else if( ( sampling_freq % 11025 )*10 < sampling_freq / 100 ) //error <= 0.1%
    {
        if( sampling_freq <= 176400 )
        {
            pll3_mul_reg = 15 ; 
            pll3_mul = 20 ;
            div = 176400 / sampling_freq * 17 ;
        }
        else
        {
            pll3_mul_reg = 12 ; 
            pll3_mul = 14 ;
            div = 6 ; 
        }
        i2s_use_pll3 = 1 ;
    }
    else if( ( 11025 - ( sampling_freq % 11025 ) )*10 < sampling_freq / 100 ) //error <= 0.1%
    {
        if( sampling_freq <= 176400 )
        {
            pll3_mul_reg = 15 ; 
            pll3_mul = 20 ;
            div = 176400 / sampling_freq * 17 ;
        }
        else
        {
            pll3_mul_reg = 12 ; 
            pll3_mul = 14 ;
            div = 6 ; 
        }
        i2s_use_pll3 = 1 ;
    }
    else
    {
        sample_rate = 48000 ; 
        div = 5 ; 
        i2s_use_pll3 = 0 ; 
    }

    if( i2s_use_pll3 ) 
    {
        //Switch OFF PLL3
        RCC->CTLR &= ~RCC_PLL3ON ;
        //PLL3MUL
        RCC->CFGR2 = ( RCC->CFGR2 & ( ~RCC_PLL3MUL ) ) | ( pll3_mul_reg << 12 );
        //Switch ON PLL3
        RCC->CTLR |= RCC_PLL3ON;
        /* Wait till PLL is ready */
        while ((RCC->CTLR & RCC_PLL2RDY) == 0)
        {
        }
        //Select I2S2 Clocks Sources
        RCC->CFGR2 |= RCC_I2S2SRC ; 
        //PLL2_DIV = RCC->CFGR2[7:4] + 1
        sample_rate = HSE_VALUE / ( ( ( RCC->CFGR2 & RCC_PREDIV2 ) >> 4 ) + 1 ) * pll3_mul / 32 / div ;
    }
    else  //use pllclk
    {
        //Select I2S2 Clocks Sources
        RCC->CFGR2 &= ~RCC_I2S2SRC ; 
        sample_rate = SystemCoreClock / 64 / div ;
    }

    if( ( SPI2->I2SCFGR & ( SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN )) == 0 )
    {   //16bits
        SPI2->I2SPR = ( SPI2->I2SPR & ( ~ ( SPI_I2SPR_I2SDIV | SPI_I2SPR_ODD ) )) | ( div & 0x00FF ) ;
    }
    else //32bits , 24biits
    {
        SPI2->I2SPR = ( SPI2->I2SPR & ( ~ ( SPI_I2SPR_I2SDIV | SPI_I2SPR_ODD ) )) | ( ( div >> 1 ) & 0x00FF ) | ( ( div & 0x01 ) << 8 );
    }

    dbg_printf("UAC Rate=%dHz,Actual Rate:%dHz\r\n",sampling_freq,sample_rate);
    feedback_rate_m = sample_rate * 1000 ;
}

uint32_t ch32v305_get_uac_feedback_freq(void)
{
    uint32_t feedback_rate ;
    feedback_rate = feedback_rate_m / 1000 ;
    return feedback_rate;
}
