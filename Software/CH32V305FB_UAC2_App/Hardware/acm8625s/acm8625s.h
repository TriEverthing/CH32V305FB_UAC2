#ifndef _USB_ACM8625S_H_
#define _USB_ACM8625S_H_

/* includes ------------------------------------------------------------------*/
#include "stdbool.h"
// #include "string.h"
// #include "stdatomic.h"
// #include "ch32v30x.h"
/* private includes ----------------------------------------------------------*/

/* exported types ------------------------------------------------------------*/
struct acm8625s_priv 
{
	uint8_t	*dsp_cfg_data;
	uint16_t dsp_cfg_len;
	uint8_t	vol[2];
	bool is_powered;
	bool is_muted;
};
/* exported macro ------------------------------------------------------------*/
/* register address */
#define REG_PAGE				0x00
#define REG_DEVICE_STATE		0x04
#define REG_STATE_REPORT		0x16
#define REG_GLOBAL_FAULT1		0x17
#define REG_GLOBAL_FAULT2		0x18
#define REG_GLOBAL_FAULT3		0x19

/* DEVICE_STATE register values */
#define DEVICE_STATE_DEEP_SLEEP	0x00
#define DEVICE_STATE_SLEEP		0x01
#define DEVICE_STATE_HIZ		0x02
#define DEVICE_STATE_PLAY		0x03

#define DEVICE_STATE_MUTE		0x0C

#define ACM_ARRAY_SIZE(arr) 	(sizeof(arr) / sizeof((arr)[0]))
/* exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void acm8625s_init(void);

void acm8625s_mute(uint8_t mute);

uint8_t acm8625s_set_vol(uint8_t vol);

void acm8625s_power_down(void);

#endif