#include "acm8625s_port.h"
#include "sf_i2c_port.h"
#include "ch32v30x_bsp.h"

void acm8625s_port_i2c_init( void )
{
    amp_i2c_init();

//SCL PB10 : push pull output
#if AMP_PDN_NUM < 8 
    AMP_PDN->CFGLR = ( AMP_PDN->CFGLR  & ( ~( (uint32_t )(0x0000000F) << ( AMP_PDN_NUM << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( AMP_PDN_NUM << 2 ) ) ;
#else
    AMP_PDN->CFGHR = ( AMP_PDN->CFGHR  & ( ~( (uint32_t )(0x0000000F) << ( ( AMP_PDN_NUM -8 )  << 2 ) ) ) )  | ( (uint32_t )(0x00000003) << ( ( AMP_PDN_NUM -8 ) << 2 ) ) ;
#endif
}

void acm8625s_port_pdn_high(void)
{
    AMP_PDN->BSHR = AMP_PDN_PIN ;
}

void acm8625s_port_pdn_low(void)
{
    AMP_PDN->BCR = AMP_PDN_PIN ;
}

void acm8625s_port_delay_ms(uint32_t dms)
{
    soft_delay_ms(dms);
}

uint8_t mcu_regmap_write(uint8_t reg, uint8_t val)
{
    i2c_Error_t i2c_err ;
    uint8_t data[2] = { reg , val };
    struct sf_i2c_msg msgs_t ;

    //target reg address
    msgs_t.addr = AMP8625S_I2CADR_8BITS ;
    msgs_t.flags = SF_I2C_FLAG_WR ;
    msgs_t.buf = data ;
    msgs_t.len = 2 ;

    // __disable_irq();
    i2c_err = sf_i2c_transfer( &amp_i2c_dev , &msgs_t , 1 );
    // __enable_irq();

    return i2c_err;
}

uint8_t mcu_regmap_read(uint8_t reg, uint8_t *val)
{
    i2c_Error_t i2c_err ;
    struct sf_i2c_msg msgs_t[2] ;

    //target reg address
    msgs_t[0].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[0].flags = SF_I2C_FLAG_WR ;
    msgs_t[0].buf = &reg ;
    msgs_t[0].len = 1 ;

    msgs_t[1].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[1].flags = SF_I2C_FLAG_RD ;
    msgs_t[1].buf = val ;
    msgs_t[1].len = 1 ;

    // __disable_irq();
    i2c_err = sf_i2c_transfer( &amp_i2c_dev , msgs_t , 2 );
    // __enable_irq();

    return i2c_err;
}

uint8_t mcu_regmap_update_bits(uint8_t reg,uint8_t mask,uint16_t val)
{
    i2c_Error_t i2c_err ;
    uint8_t reg_t ;

    mcu_regmap_read( reg , &reg_t );
    reg_t = reg_t & ( ~ mask ) ;

    i2c_err = mcu_regmap_write( reg , reg_t );

    return i2c_err;
}

uint8_t mcu_regmap_bulk_write(uint8_t reg, const void *val,size_t val_count)
{
    i2c_Error_t i2c_err ;
    struct sf_i2c_msg msgs_t[2] ;

    if (val_count==0) 
        return 0;

    //target reg address
    msgs_t[0].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[0].flags = SF_I2C_FLAG_WR ;
    msgs_t[0].buf = &reg ;
    msgs_t[0].len = 1 ;

    msgs_t[1].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[1].flags = SF_I2C_FLAG_WR | SF_I2C_FLAG_NO_START ;
    msgs_t[1].buf = (uint8_t *)val ;
    msgs_t[1].len = val_count ;

    // __disable_irq();
    i2c_err = sf_i2c_transfer( &amp_i2c_dev , msgs_t , 2 );
    // __enable_irq();

    return i2c_err;
}

uint8_t mcu_regmap_bulk_read(uint8_t reg, void *val,size_t val_count)
{
    i2c_Error_t i2c_err ;
    struct sf_i2c_msg msgs_t[2] ;

    if (val_count==0) 
        return 0;

    //target reg address
    msgs_t[0].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[0].flags = SF_I2C_FLAG_WR ;
    msgs_t[0].buf = &reg ;
    msgs_t[0].len = 1 ;

    msgs_t[1].addr = AMP8625S_I2CADR_8BITS ;
    msgs_t[1].flags = SF_I2C_FLAG_RD ;
    msgs_t[1].buf = val ;
    msgs_t[1].len = val_count ;

    // __disable_irq();
    i2c_err = sf_i2c_transfer( &amp_i2c_dev , msgs_t , 2 );
    // __enable_irq();

    return i2c_err;
}
