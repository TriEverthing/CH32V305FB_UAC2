#ifndef _SF_I2C_PORT_H_
#define _SF_I2C_PORT_H_

/* includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdatomic.h"
#include "sf_i2c.h"
#include "ch32v30x.h"
/* private includes ----------------------------------------------------------*/

/* exported macro ------------------------------------------------------------*/

/* exported macro ------------------------------------------------------------*/


/* exported constants --------------------------------------------------------*/

/* exported types ------------------------------------------------------------*/
extern struct sf_i2c_dev amp_i2c_dev;
/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void amp_i2c_init(void);

#endif