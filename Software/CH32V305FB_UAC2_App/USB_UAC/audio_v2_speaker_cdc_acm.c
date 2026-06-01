/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
//stdlib
#include "stdio.h"
#include "stdarg.h"
//cherry usb
#include "usbd_core.h"
#include "usbd_audio.h"
#include "usb_cdc.h"
#include "usbd_cdc_acm.h"
//
#include "usb_i2s.h"

#define USING_FEEDBACK              1

#define USBD_VID                    0x1A86 //Nanjing Qinheng
#define USBD_PID                    0x0001 //
#define USBD_MAX_POWER              100
#define USBD_LANGID_STRING          1033

#ifdef CONFIG_USB_HS
#if AUDIO_OUT_PACKET <= 1024
#define AUDIO_EP_OUT_SIZE           AUDIO_OUT_PACKET
#define EP_INTERVAL                 0x04
#define FEEDBACK_ENDP_PACKET_SIZE   0x04
#else 
#define AUDIO_EP_OUT_SIZE           1024
#define EP_INTERVAL                 0x02
#define FEEDBACK_ENDP_PACKET_SIZE   0x04
#endif
#else
#define AUDIO_EP_OUT_SIZE           AUDIO_OUT_PACKET
#define EP_INTERVAL                 0x01
#define FEEDBACK_ENDP_PACKET_SIZE   0x03
#endif

/*!< audio endpoint address */
#define AUDIO_OUT_EP                0x01
#define AUDIO_OUT_FEEDBACK_EP       0x82

/*!< cdc acm endpoint address */
#define CDC_OUT_EP                  0x03
#define CDC_IN_EP                   0x83
#define CDC_INT_EP                  0x84

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif


#define AUDIO_OUT_CLOCK_ID          0x03
#define AUDIO_IN_THERMINAL_ID       0x01
#define AUDIO_OUT_FU_ID             0x04
#define AUDIO_OUT_THERMINAL_ID      0x02

#define BMCONTROL                   (AUDIO_V2_FU_CONTROL_MUTE | AUDIO_V2_FU_CONTROL_VOLUME)

#if OUT_CHANNEL_NUM == 1
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x00000000
#elif OUT_CHANNEL_NUM == 2
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE ( AUDIO_CHANNEL_FL | AUDIO_CHANNEL_FR )
#elif OUT_CHANNEL_NUM == 3
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x00000007
#elif OUT_CHANNEL_NUM == 4
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000000f
#elif OUT_CHANNEL_NUM == 5
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000001f
#elif OUT_CHANNEL_NUM == 6
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000003F
#elif OUT_CHANNEL_NUM == 7
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x0000007f
#elif OUT_CHANNEL_NUM == 8
#define OUTPUT_CTRL      DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL), DBVAL(BMCONTROL)
#define OUTPUT_CH_ENABLE 0x000000ff
#endif

#if USING_FEEDBACK == 0
#define USB_AUDIO_CONFIG_DESC_SIZ (9 +                                                     \
                                   AUDIO_V2_AC_DESCRIPTOR_INIT_LEN +                       \
                                   AUDIO_V2_SIZEOF_AC_CLOCK_SOURCE_DESC +                  \
                                   AUDIO_V2_SIZEOF_AC_INPUT_TERMINAL_DESC +                \
                                   AUDIO_V2_SIZEOF_AC_FEATURE_UNIT_DESC(OUT_CHANNEL_NUM) + \
                                   AUDIO_V2_SIZEOF_AC_OUTPUT_TERMINAL_DESC +               \
                                   AUDIO_V2_AS_DESCRIPTOR_INIT_LEN +                        \
                                   CDC_ACM_DESCRIPTOR_LEN )
#else
#define USB_AUDIO_CONFIG_DESC_SIZ (9 +                                                     \
                                   AUDIO_V2_AC_DESCRIPTOR_INIT_LEN +                       \
                                   AUDIO_V2_SIZEOF_AC_CLOCK_SOURCE_DESC +                  \
                                   AUDIO_V2_SIZEOF_AC_INPUT_TERMINAL_DESC +                \
                                   AUDIO_V2_SIZEOF_AC_FEATURE_UNIT_DESC(OUT_CHANNEL_NUM) + \
                                   AUDIO_V2_SIZEOF_AC_OUTPUT_TERMINAL_DESC +               \
                                   AUDIO_V2_AS_FEEDBACK_DESCRIPTOR_INIT_LEN +              \
                                   CDC_ACM_DESCRIPTOR_LEN )
#endif

#define AUDIO_AC_SIZ (AUDIO_V2_SIZEOF_AC_HEADER_DESC +                        \
                      AUDIO_V2_SIZEOF_AC_CLOCK_SOURCE_DESC +                  \
                      AUDIO_V2_SIZEOF_AC_INPUT_TERMINAL_DESC +                \
                      AUDIO_V2_SIZEOF_AC_FEATURE_UNIT_DESC(OUT_CHANNEL_NUM) + \
                      AUDIO_V2_SIZEOF_AC_OUTPUT_TERMINAL_DESC)

#ifdef CONFIG_USBDEV_ADVANCE_DESC
static const uint8_t device_descriptor[] = 
{
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01 , USBD_VID, USBD_PID, 0x0009 , 0x01)
};
static const uint8_t config_descriptor[] = 
{
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* USB audio V2.0 */
    AUDIO_V2_AC_DESCRIPTOR_INIT(0x00, 0x02, AUDIO_AC_SIZ, AUDIO_CATEGORY_SPEAKER, 0x00, 0x00),
    AUDIO_V2_AC_CLOCK_SOURCE_DESCRIPTOR_INIT(AUDIO_OUT_CLOCK_ID, 0x02, 0x03),
    AUDIO_V2_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(AUDIO_IN_THERMINAL_ID, AUDIO_TERMINAL_STREAMING, AUDIO_OUT_CLOCK_ID, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, 0x0000),
    AUDIO_V2_AC_FEATURE_UNIT_DESCRIPTOR_INIT(AUDIO_OUT_FU_ID, AUDIO_IN_THERMINAL_ID, OUTPUT_CTRL),
    AUDIO_V2_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(AUDIO_OUT_THERMINAL_ID, AUDIO_OUTTERM_SPEAKER, AUDIO_OUT_FU_ID, AUDIO_OUT_CLOCK_ID, 0x0000),
#if USING_FEEDBACK == 0
    AUDIO_V2_AS_DESCRIPTOR_INIT(0x01, AUDIO_IN_THERMINAL_ID, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, HALF_WORD_BYTES, SAMPLE_BITS, AUDIO_OUT_EP, 0x09, AUDIO_OUT_PACKET, EP_INTERVAL),
#else
    AUDIO_V2_AS_FEEDBACK_DESCRIPTOR_INIT(0x01, AUDIO_IN_THERMINAL_ID, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, HALF_WORD_BYTES, SAMPLE_BITS, AUDIO_OUT_EP, AUDIO_OUT_PACKET, EP_INTERVAL, AUDIO_OUT_FEEDBACK_EP),
#endif
    /* USB CDC ACM */
    CDC_ACM_DESCRIPTOR_INIT( 0x02 , CDC_INT_EP , CDC_OUT_EP , CDC_IN_EP , CDC_MAX_MPS , 0x00 )
};

static const uint8_t device_quality_descriptor[] = 
{
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
};

static const char *string_descriptors[] = 
{
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "WCH-USB",                  /* Manufacturer */
    "WCH-USB-UAC2",             /* Product */
    "2022123456",               /* Serial Number */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    if (index > 3) 
    {
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor audio_v2_cdc_acm_descriptor = 
{
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};
#else
const uint8_t audio_v2_cdc_acm_descriptor[] = 
{
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0001, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),

    /* USB audio V2.0 */
    AUDIO_V2_AC_DESCRIPTOR_INIT(0x00, 0x02, AUDIO_AC_SIZ, AUDIO_CATEGORY_SPEAKER, 0x00, 0x00),
    AUDIO_V2_AC_CLOCK_SOURCE_DESCRIPTOR_INIT(AUDIO_OUT_CLOCK_ID, 0x03, 0x03),
    AUDIO_V2_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x02, AUDIO_TERMINAL_STREAMING, 0x01, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, 0x0000),
    AUDIO_V2_AC_FEATURE_UNIT_DESCRIPTOR_INIT(AUDIO_OUT_FU_ID, 0x02, OUTPUT_CTRL),
    AUDIO_V2_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x04, AUDIO_OUTTERM_SPEAKER, 0x03, 0x01, 0x0000),
#if USING_FEEDBACK == 0
    AUDIO_V2_AS_DESCRIPTOR_INIT(0x01, 0x02, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, HALF_WORD_BYTES, SAMPLE_BITS, AUDIO_OUT_EP, 0x09, AUDIO_OUT_PACKET, EP_INTERVAL),
#else
    AUDIO_V2_AS_FEEDBACK_DESCRIPTOR_INIT(0x01, 0x02, OUT_CHANNEL_NUM, OUTPUT_CH_ENABLE, HALF_WORD_BYTES, SAMPLE_BITS, AUDIO_OUT_EP, AUDIO_OUT_PACKET, EP_INTERVAL, AUDIO_OUT_FEEDBACK_EP),
#endif

    /* USB CDC ACM */
    CDC_ACM_DESCRIPTOR_INIT( 0x02 , CDC_INT_EP , CDC_OUT_EP , CDC_IN_EP , CDC_MAX_MPS , 0x00 )

    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x10,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'W', 0x00,                  /* wcChar0 */
    'C', 0x00,                  /* wcChar1 */
    'H', 0x00,                  /* wcChar2 */
    '-', 0x00,                  /* wcChar3 */
    'U', 0x00,                  /* wcChar4 */
    'S', 0x00,                  /* wcChar5 */
    'B', 0x00,                  /* wcChar6 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x1A,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'W', 0x00,                  /* wcChar0 */
    'C', 0x00,                  /* wcChar1 */
    'H', 0x00,                  /* wcChar2 */
    '-', 0x00,                  /* wcChar3 */
    'U', 0x00,                  /* wcChar4 */
    'S', 0x00,                  /* wcChar5 */
    'B', 0x00,                  /* wcChar6 */
    '-', 0x00,                  /* wcChar7 */
    'U', 0x00,                  /* wcChar8 */
    'A', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    '2', 0x00,                  /* wcChar11 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
#if USING_FEEDBACK == 0
    '3', 0x00,                  /* wcChar9 */
#else
    '4', 0x00, /* wcChar9 */
#endif
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
#endif
    0x00
};
#endif

/*
*   for example HSE=8MHz
*   8kHz , CH32V305 : PLL3_DIV=5,PLL2_MUL=16,I2S_DIV=50,VCO=25.6MHz,freq=8kHz,0%
*   11.025kHz , CH32V305 : PLL3_DIV=2,PLL2_MUL=12,I2S_DIV=68,VCO=48MHz,freq=11.029kHz,0.04%
*   16kHz , CH32V305 : PLL3_DIV=5,PLL2_MUL=16,I2S_DIV=25,VCO=25.6MHz,freq=16kHz,0%
*   22.050kHz = 2*11.025kHz , CH32V305 : PLL3_DIV=2,PLL2_MUL=12,I2S_DIV=34,VCO=48MHz,freq=22.058kHz,0.04%
*   44.1kHz = 4*11.025kHz , CH32V305 : PLL3_DIV=2,PLL2_MUL=12,I2S_DIV=17,VCO=48MHz,freq=44.117kHz,0.04%
*   48kHz , CH32V305 : PLL3_DIV=2,PLL2_MUL=20,I2S_DIV=26,VCO=80MHz,freq=48.077kHz,0.16026%
*   88.2kHz = 8*11.025kHz
*   96kHz = 2*48kHz
*   176.4kHz = 16*11.025kHz
*   192kHz = 4*48kHz
*   352.8kHz = 32*11.025kHz
*   384kHz = 8*48kHz
*/
static const uint8_t default_sampling_freq_table[] = 
{
    AUDIO_SAMPLE_FREQ_NUM( audio_max_num ),
    AUDIO_SAMPLE_FREQ_4B(8000),     //8kHz
    AUDIO_SAMPLE_FREQ_4B(8000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#if AUDIO_OUT_MAX_FREQ >= 11025
    AUDIO_SAMPLE_FREQ_4B(11025),    //11.025kHz
    AUDIO_SAMPLE_FREQ_4B(11025),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 16000
    AUDIO_SAMPLE_FREQ_4B(16000),    //16kHz
    AUDIO_SAMPLE_FREQ_4B(16000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 22050
    AUDIO_SAMPLE_FREQ_4B(22050),    //22.05kHz
    AUDIO_SAMPLE_FREQ_4B(22050),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 32000
    AUDIO_SAMPLE_FREQ_4B(32000),    //32kHz
    AUDIO_SAMPLE_FREQ_4B(32000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 44100
    AUDIO_SAMPLE_FREQ_4B(44100),    //44.1kHz
    AUDIO_SAMPLE_FREQ_4B(44100),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 48000
    AUDIO_SAMPLE_FREQ_4B(48000),    //48kHz
    AUDIO_SAMPLE_FREQ_4B(48000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 88200
    AUDIO_SAMPLE_FREQ_4B(88200),    //88.2kHz
    AUDIO_SAMPLE_FREQ_4B(88200),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 96000
    AUDIO_SAMPLE_FREQ_4B(96000),    //96kHz
    AUDIO_SAMPLE_FREQ_4B(96000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 176000
    AUDIO_SAMPLE_FREQ_4B(176400),   //176.4kHz
    AUDIO_SAMPLE_FREQ_4B(176400),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 192000
    AUDIO_SAMPLE_FREQ_4B(192000),   //192kHz
    AUDIO_SAMPLE_FREQ_4B(192000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 352800
    AUDIO_SAMPLE_FREQ_4B(352800),   //352.8kHz
    AUDIO_SAMPLE_FREQ_4B(352800),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
#if AUDIO_OUT_MAX_FREQ >= 384000
    AUDIO_SAMPLE_FREQ_4B(384000),   //384kHz
    AUDIO_SAMPLE_FREQ_4B(384000),
    AUDIO_SAMPLE_FREQ_4B(0x00),
#endif
};

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t uac_read_buffer[AUDIO_OUT_PACKET];
#if USING_FEEDBACK == 1
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t s_speaker_feedback_buffer[4];
#endif

volatile bool uac_rx_flag = 0;
volatile uint32_t s_speaker_sample_rate;

#define USB_VCP_RX_BUF_SIZE 1024
#define USB_VCP_TX_BUF_SIZE 512
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t acm_read_buffer[USB_VCP_RX_BUF_SIZE]; /* 2048 is only for test speed , please use CDC_MAX_MPS for common*/
//usb vcp tx 
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t acm_write_buffer1[USB_VCP_TX_BUF_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t acm_write_buffer2[USB_VCP_TX_BUF_SIZE];
uint8_t * acm_write_buffer = acm_write_buffer1 ;
uint16_t acm_tx_ptr = 0 ;
static uint8_t acm_tx_buf_idle_flag = 1;
static uint8_t acm_tx_busy_flag = 0;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) 
    {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            acm_tx_buf_idle_flag = 1;
            acm_tx_busy_flag = 0 ;
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, CDC_OUT_EP, acm_read_buffer, USB_VCP_RX_BUF_SIZE);
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;
        default:
            break;
    }
}

void usbd_audio_open(uint8_t busid, uint8_t intf)
{
    uac_rx_flag = 1;
    ch32_i2s_start();
    /* setup first out ep read transfer */
    usbd_ep_start_read( busid , AUDIO_OUT_EP , uac_read_buffer , AUDIO_OUT_PACKET );
    uint32_t feedback_value ;
    feedback_value = ch32v305_get_uac_feedback_freq();
    //feedback_value = s_speaker_sample_rate ;
#if USING_FEEDBACK == 1
#ifdef CONFIG_USB_HS
    //period = 2 ^( EP_INTERVAL - 4 ) ms = 250us
    feedback_value = AUDIO_FREQ_TO_FEEDBACK_HS(feedback_value)>>( 4 - EP_INTERVAL );
    AUDIO_FEEDBACK_TO_BUF_HS(s_speaker_feedback_buffer, feedback_value);
#else
    feedback_value = AUDIO_FREQ_TO_FEEDBACK_FS(feedback_value);
    AUDIO_FEEDBACK_TO_BUF_FS(s_speaker_feedback_buffer, feedback_value);
#endif
    usbd_ep_start_write(busid, AUDIO_OUT_FEEDBACK_EP, s_speaker_feedback_buffer, FEEDBACK_ENDP_PACKET_SIZE);
#endif
    // USB_LOG_RAW("OPEN\r\n");
}

void usbd_audio_close(uint8_t busid, uint8_t intf)
{
    // USB_LOG_RAW("CLOSE\r\n");
    ch32_i2s_stop();
    uac_rx_flag = 0;
}

void usbd_audio_set_sampling_freq(uint8_t busid, uint8_t ep, uint32_t sampling_freq)
{
    (void)busid;
    if (ep == AUDIO_OUT_EP) 
    {
        ch32v305_set_i2s_freq( sampling_freq );        
        s_speaker_sample_rate = sampling_freq ;
    }
}

uint32_t usbd_audio_get_sampling_freq(uint8_t busid, uint8_t ep)
{
    (void)busid;
    if (ep == AUDIO_OUT_EP) 
    {
        s_speaker_sample_rate = ch32v305_get_uac_feedback_freq();
    }
    return s_speaker_sample_rate ;
}

void usbd_audio_get_sampling_freq_table(uint8_t busid, uint8_t ep, uint8_t **sampling_freq_table)
{
    if (ep == AUDIO_OUT_EP) 
    {
        *sampling_freq_table = (uint8_t *)default_sampling_freq_table;
    }
}

void usbd_audio_iso_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    //USB_LOG_RAW("actual out len:%d\r\n", (unsigned int)nbytes);
    ch32_i2s_write( uac_read_buffer , nbytes  );
    usbd_ep_start_read(busid, AUDIO_OUT_EP, uac_read_buffer , AUDIO_OUT_PACKET);
}

#if USING_FEEDBACK == 1
void usbd_audio_iso_out_feedback_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    //USB_LOG_RAW("actual feedback len:%d\r\n", (unsigned int)nbytes);
    uint32_t feedback_value ;
    feedback_value = ch32v305_get_uac_feedback_freq();
    //feedback_value = s_speaker_sample_rate ;
#ifdef CONFIG_USB_HS
    feedback_value = AUDIO_FREQ_TO_FEEDBACK_HS(feedback_value)>>( 4 - EP_INTERVAL );
    AUDIO_FEEDBACK_TO_BUF_HS(s_speaker_feedback_buffer, feedback_value);
#else
    feedback_value = AUDIO_FREQ_TO_FEEDBACK_FS(feedback_value);
    AUDIO_FEEDBACK_TO_BUF_FS(s_speaker_feedback_buffer, feedback_value);
#endif
    usbd_ep_start_write(busid, AUDIO_OUT_FEEDBACK_EP, s_speaker_feedback_buffer, FEEDBACK_ENDP_PACKET_SIZE);
}
#endif

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    //USB_LOG_RAW("actual out len:%d\r\n", (unsigned int)nbytes);
    /* setup next out ep read transfer */
    usbd_ep_start_read(busid, CDC_OUT_EP, acm_read_buffer, USB_VCP_RX_BUF_SIZE );
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    //USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    if( acm_tx_ptr > 0 && acm_tx_buf_idle_flag ) //cdc_tx_ptr > 0  and ex_tx_status buffer filled
    {
        usbd_ep_start_write( 0 , CDC_IN_EP , acm_write_buffer, acm_tx_ptr );
        acm_tx_ptr = 0 ;

        if( acm_write_buffer == acm_write_buffer1 )
            acm_write_buffer = acm_write_buffer2 ;
        else
            acm_write_buffer = acm_write_buffer1 ;        
    }
    else
    {
        if((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) 
        {
            /* send zlp */
            usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0);
        } 
        else 
        {
            acm_tx_busy_flag = 0 ; //IDLE
        }        
    }
}

/*!< usb audio endpoint call back */
static struct usbd_endpoint audio_out_ep = 
{
    .ep_cb = usbd_audio_iso_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

#if USING_FEEDBACK == 1
static struct usbd_endpoint audio_out_feedback_ep = 
{
    .ep_cb = usbd_audio_iso_out_feedback_callback,
    .ep_addr = AUDIO_OUT_FEEDBACK_EP
};
#endif

/*!< cdc acm endpoint call back */
struct usbd_endpoint cdc_out_ep = 
{
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = 
{
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

/*!< usb audio interface */
struct usbd_interface intf0;
struct usbd_interface intf1;
/*!< cdc acm interface */
struct usbd_interface intf2;
struct usbd_interface intf3;

struct audio_entity_info audio_entity_table[] = 
{
    { 
        .bEntityId = AUDIO_OUT_CLOCK_ID,
        .bDescriptorSubtype = AUDIO_CONTROL_CLOCK_SOURCE,
        .ep = AUDIO_OUT_EP 
    },

    { 
        .bEntityId = AUDIO_OUT_FU_ID,
        .bDescriptorSubtype = AUDIO_CONTROL_FEATURE_UNIT,
        .ep = AUDIO_OUT_EP 
    },
};

void audio_v2_init(uint8_t busid, uintptr_t reg_base)
{
    acm_tx_buf_idle_flag = 1 ;
    acm_tx_busy_flag = 0 ;

#ifdef CONFIG_USBDEV_ADVANCE_DESC
    usbd_desc_register(busid, &audio_v2_cdc_acm_descriptor);
#else
    usbd_desc_register(busid, audio_v2_cdc_acm_descriptor);
#endif

    usbd_add_interface(busid, usbd_audio_init_intf(busid, &intf0, 0x0200, audio_entity_table, 2));
    usbd_add_interface(busid, usbd_audio_init_intf(busid, &intf1, 0x0200, audio_entity_table, 2));
    usbd_add_endpoint(busid, &audio_out_ep);
#if USING_FEEDBACK == 1
    usbd_add_endpoint(busid, &audio_out_feedback_ep);
#endif

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf2));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf3));
    usbd_add_endpoint(busid, &cdc_out_ep);
    usbd_add_endpoint(busid, &cdc_in_ep);

    usbd_initialize(busid, reg_base, usbd_event_handler);
}

volatile uint8_t dtr_enable = 0;

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    if (dtr) 
    {
        dtr_enable = 1;
    } 
    else 
    {
        dtr_enable = 0;
    }
}

void usb_vcp_refresh_buf(void)
{
	if( dtr_enable == 0 )
		return ;

	if( acm_tx_ptr > 0 && acm_tx_busy_flag == 0 ) 
    {
		acm_tx_busy_flag = 1 ;
        usbd_ep_start_write( 0 , CDC_IN_EP , acm_write_buffer, acm_tx_ptr );
        acm_tx_ptr = 0 ;

        if( acm_write_buffer == acm_write_buffer1 )
            acm_write_buffer = acm_write_buffer2 ;
        else
            acm_write_buffer = acm_write_buffer1 ;        
    }
}

uint16_t usb_vcp_printf(const char* fmt,...)
{
    va_list ap ;
    uint16_t len ;
	acm_tx_buf_idle_flag = 0 ;
    //__disable_irq();
    len = USB_VCP_TX_BUF_SIZE - acm_tx_ptr ;
	va_start(ap,fmt);	
    len = vsnprintf((char*)( acm_write_buffer + acm_tx_ptr ) , len , fmt , ap );
    va_end(ap);
    acm_tx_ptr = acm_tx_ptr + len + 1 ;
	//__enable_irq();
	usb_vcp_refresh_buf();
	acm_tx_buf_idle_flag = 1 ;
	return len;
}

uint16_t usb_vcp_write_buf(const char* data , uint16_t length )
{
    uint16_t len ;
	acm_tx_buf_idle_flag = 0 ;
    //__disable_irq();
    len = USB_VCP_TX_BUF_SIZE - acm_tx_ptr ;
    len = length > len ? len : length ;
    memcpy( acm_write_buffer + acm_tx_ptr , data , len );
    acm_tx_ptr = acm_tx_ptr + len ;
	//__enable_irq();
	usb_vcp_refresh_buf();
	acm_tx_buf_idle_flag = 1 ;
    return len ;
}

void audio_v2_test(uint8_t busid)
{
    if (uac_rx_flag) 
    {

    }
}
