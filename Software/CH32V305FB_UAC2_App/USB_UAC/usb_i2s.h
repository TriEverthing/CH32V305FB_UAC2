#ifndef _USB_I2S_H_
#define _USB_I2S_H_

/* includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdatomic.h"

#include "ch32v30x_bsp.h"
/* private includes ----------------------------------------------------------*/

/* exported macro ------------------------------------------------------------*/
#define ENBALE_I2S_MSCK             0
/* exported macro ------------------------------------------------------------*/
#define AUDIO_OUT_MAX_FREQ          384000
//bBitResolution ¡Ü bSubslotSize*8.
#define OUT_CHANNEL_NUM             2
#define SAMPLE_BITS                 32 //16 bit per channel , The number of effectively used bits from the available bits in an audio subslot.
#define AUDIO_OUT_PACKET            (((AUDIO_OUT_MAX_FREQ * HALF_WORD_BYTES * OUT_CHANNEL_NUM) / 1000))
#if SAMPLE_BITS == 16
#define HALF_WORD_BYTES             2  //The number of bytes occupied by one audio subslot. Can be 1, 2, 3 or 4.
#elif SAMPLE_BITS == 24
#define HALF_WORD_BYTES             3  //The number of bytes occupied by one audio subslot. Can be 1, 2, 3 or 4.
#elif SAMPLE_BITS == 32
#define HALF_WORD_BYTES             4  //The number of bytes occupied by one audio subslot. Can be 1, 2, 3 or 4.
#else
#error "Not Support audio sample bits."
#endif

/* exported constants --------------------------------------------------------*/

/* exported types ------------------------------------------------------------*/
typedef struct _StereoAudio
{
#if SAMPLE_BITS == 16
    uint16_t left;
    uint16_t right;
#else
    uint32_t left;
    uint32_t right;
#endif
} StereoAudio_t;

typedef enum
{
    ch32_i2s_16bits = 0 ,
    ch32_i2s_24bits ,
    ch32_i2s_32bits ,
}ch32_i2s_width;

typedef enum
{
    ch32_i2s_8kHz = 0 ,
#if AUDIO_OUT_MAX_FREQ >= 11025
    ch32_i2s_11p025kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 16000
    ch32_i2s_16kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 22050
    ch32_i2s_22p05kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 32000
    ch32_i2s_32kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 44100
    ch32_i2s_44p1kHz ,
    #endif
#if AUDIO_OUT_MAX_FREQ >= 48000
    ch32_i2s_48kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 88200
    ch32_i2s_88p2kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 96000
    ch32_i2s_96kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 176400
    ch32_i2s_176p4kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 192000
    ch32_i2s_192kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 352800
    ch32_i2s_352p8kHz ,
#endif
#if AUDIO_OUT_MAX_FREQ >= 384000
    ch32_i2s_384kHz ,
#endif
    audio_max_num ,
}ch32_i2s_freq;
/* Exported macro ------------------------------------------------------------*/
extern uint32_t max_uac_len_ever;
extern uint32_t min_uac_len_ever;
extern uint32_t mesured_dma_sample_rate;
extern uint32_t mesured_usb_sample_rate;
extern float report_fs;
/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void usb_i2s_init(void);

void ch32v305_set_i2s_freq(uint32_t sampling_freq);

uint32_t ch32v305_get_uac_feedback_freq(void);

void ch32_i2s_start(void);

void ch32_i2s_stop(void);

void ch32_i2s_write(uint8_t* ptr, uint32_t len);

extern uint16_t usb_vcp_printf(const char* fmt,...);

#endif