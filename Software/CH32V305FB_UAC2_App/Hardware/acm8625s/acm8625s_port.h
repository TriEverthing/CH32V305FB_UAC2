#ifndef _ACM8625S_PORT_H_
#define _ACM8625S_PORT_H_

/* includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
/* private includes ----------------------------------------------------------*/

/* exported types ------------------------------------------------------------*/

/* exported macro ------------------------------------------------------------*/
#define AMP8625S_I2CADR_8BITS       0x58  
// #define AMP8625S_I2CADR_8BITS       0x5A
// #define AMP8625S_I2CADR_8BITS       0x5C
// #define AMP8625S_I2CADR_8BITS       0x5E
/* exported macro ------------------------------------------------------------*/
#define AMP_PDN             GPIOA
#define AMP_PDN_RCC_EN      RCC_IOPAEN
#define AMP_PDN_NUM         5
#define AMP_PDN_PIN         (((uint16_t)0x0001) << AMP_PDN_NUM )
/* exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void acm8625s_port_i2c_init( void );

void acm8625s_port_pdn_high(void);

void acm8625s_port_pdn_low(void);

void acm8625s_port_delay_ms(uint32_t dms);

uint8_t mcu_regmap_write(uint8_t reg, uint8_t val);

uint8_t mcu_regmap_read(uint8_t reg, uint8_t *val);

uint8_t mcu_regmap_update_bits(uint8_t reg,uint8_t mask,uint16_t val);

uint8_t mcu_regmap_bulk_write(uint8_t reg, const void *val,size_t val_count);

uint8_t mcu_regmap_bulk_read(uint8_t reg, void *val,size_t val_count);


#endif