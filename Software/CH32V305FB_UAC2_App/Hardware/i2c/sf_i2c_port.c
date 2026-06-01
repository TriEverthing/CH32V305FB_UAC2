///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <AMP_io.c>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2019/12/10
//   @fileversion: AMP_EVB_1.32
//******************************************/
#include "ch32v30x_bsp.h"
#include "sf_i2c.h"

// include --- headers for I/O
unsigned char g_i2c_dev = 1;
// include --- headers for I/O

#define AMP_SCL             GPIOB
#define AMP_SCL_RCC_EN      RCC_IOPBEN
#define AMP_SCL_NUM         10
#define AMP_SCL_PIN         (((uint16_t)0x0001) << AMP_SCL_NUM )

#define AMP_SDA             GPIOB
#define AMP_SDA_RCC_EN      RCC_IOPBEN
#define AMP_SDA_NUM         11
#define AMP_SDA_PIN         (((uint16_t)0x0001) << AMP_SDA_NUM )


/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void set_scl_low( void )
{
    AMP_SCL->BCR = AMP_SCL_PIN ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void set_scl_high( void )
{
    AMP_SCL->BSHR = AMP_SCL_PIN ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void set_sda_low( void )
{
    AMP_SDA->BCR = AMP_SDA_PIN ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void set_sda_high( void )
{
    AMP_SDA->BSHR = AMP_SDA_PIN ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
uint8_t read_sda( void )
{
   return ( ( AMP_SDA->INDR & AMP_SDA_PIN ) >> AMP_SDA_NUM ) ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void sda_dir_input( void )
{
    //SDA PB11 : pull up input
#if AMP_SDA_NUM < 8 
    AMP_SDA->CFGLR = ( AMP_SDA->CFGLR  & ( ~( (uint32_t )(0x0000000F) << ( AMP_SDA_NUM << 2 ) ) ) )  | ( (uint32_t )(0x00000008) << ( AMP_SDA_NUM << 2 ) ) ;
#else
    AMP_SDA->CFGHR = ( AMP_SDA->CFGHR  & ( ~( (uint32_t )(0x0000000F) << ( ( AMP_SDA_NUM -8 )  << 2 ) ) ) )  | ( (uint32_t )(0x00000008) << ( ( AMP_SDA_NUM -8 ) << 2 ) ) ;
#endif
    AMP_SDA->BSHR = AMP_SDA_PIN ;
}

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void sda_dir_output( void )
{
//SDA PB11 : push pull output
#if AMP_SDA_NUM < 8 
    AMP_SDA->CFGLR = ( AMP_SDA->CFGLR  & ( ~( (uint32_t )(0x0000000F) << ( AMP_SDA_NUM << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( AMP_SDA_NUM << 2 ) ) ;
#else
    AMP_SDA->CFGHR = ( AMP_SDA->CFGHR  & ( ~( (uint32_t )(0x0000000F) << ( ( AMP_SDA_NUM -8 )  << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( ( AMP_SDA_NUM -8 ) << 2 ) ) ;
#endif
}


/* Exported types ------------------------------------------------------------*/
struct sf_i2c_dev amp_i2c_dev = 
{
#if (I2C_DEV_FIND > 0u)
    .name               = "i2c0",
#endif
    .speed              = 30, /*! speed:105Hz */
    .delay_us           = soft_delay_us ,
    .ops.sda_low        = set_sda_low ,
    .ops.sda_high       = set_sda_high ,
    .ops.scl_low        = set_scl_low ,
    .ops.scl_high       = set_scl_high ,
    .ops.sda_read_level = read_sda ,
    .ops.sda_set_input  = sda_dir_input ,
    .ops.sda_set_output = sda_dir_output ,
};

/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void amp_i2c_init(void)
{
    static uint8_t amp_i2c_initialized = 0 ;

    if( amp_i2c_initialized )
        return;
        
    //Enable GPIO Clocks
    RCC->APB2PCENR |= AMP_SCL_RCC_EN | AMP_SDA_RCC_EN ;
    //initial gpio idle state
    AMP_SCL->BSHR = AMP_SCL_PIN ;
    AMP_SDA->BSHR = AMP_SDA_PIN ;
    //SCL PB10 : push pull output
    #if AMP_SCL_NUM < 8 
        AMP_SCL->CFGLR = ( AMP_SCL->CFGLR  & ( ~( (uint32_t )(0x0000000F) << ( AMP_SCL_NUM << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( AMP_SCL_NUM << 2 ) ) ;
    #else
        AMP_SCL->CFGHR = ( AMP_SCL->CFGHR  & ( ~( (uint32_t )(0x0000000F) << ( ( AMP_SCL_NUM -8 )  << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( ( AMP_SCL_NUM -8 ) << 2 ) ) ;
    #endif
    //SDA PB11 : open drain output
    #if AMP_SDA_NUM < 8 
        AMP_SDA->CFGLR = ( AMP_SDA->CFGLR  & ( ~( (uint32_t )(0x0000000F) << ( AMP_SDA_NUM << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( AMP_SDA_NUM << 2 ) ) ;
    #else
        AMP_SDA->CFGHR = ( AMP_SDA->CFGHR  & ( ~( (uint32_t )(0x0000000F) << ( ( AMP_SDA_NUM -8 )  << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( ( AMP_SDA_NUM -8 ) << 2 ) ) ;
    #endif
    sf_i2c_init(&amp_i2c_dev);

    amp_i2c_initialized = 1 ;
}
