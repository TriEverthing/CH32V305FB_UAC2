/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "usbd_msc.h"

#include "bootuf2.h"

#include "ch32v30x.h"
#include "ch32v30x_flash.h"


/*!< endpoint address */
#define CDC_IN_EP               0x83
#define CDC_OUT_EP              0x04
#define CDC_INT_EP              0x85

#define MSC_IN_EP               0x81
#define MSC_OUT_EP              0x02

#define USBD_VID                0x239A
#define USBD_PID                0x005D
#define USBD_MAX_POWER          100
#define USBD_LANGID_STRING      1033

#define USB_CONFIG_SIZE (9 + MSC_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#ifdef CONFIG_USB_HS
#define MSC_MAX_MPS 512
#else
#define MSC_MAX_MPS 64
#endif

#ifdef CONFIG_USBDEV_ADVANCE_DESC
static const uint8_t device_descriptor[] = 
{
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01)
};

static const uint8_t config_descriptor[] = 
{
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02)
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

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "CherryUSB",                  /* Manufacturer */
    "CherryUSB UF2 DEMO",         /* Product */
    "2022123456",                 /* Serial Number */
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
    if (index > 3) {
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor msc_bootuf2_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};
#else
const uint8_t msc_bootuf2_descriptor[] = 
{
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
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
    0x22,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'D', 0x00,                  /* wcChar0 */
    'E', 0x00,                  /* wcChar1 */
    'B', 0x00,                  /* wcChar2 */
    'U', 0x00,                  /* wcChar3 */
    'G', 0x00,                  /* wcChar4 */
    '_', 0x00,                  /* wcChar5 */
    'H', 0x00,                  /* wcChar6 */
    'D', 0x00,                  /* wcChar7 */
    'M', 0x00,                  /* wcChar8 */
    'I', 0x00,                  /* wcChar9 */
    '_', 0x00,                  /* wcChar10 */
    'T', 0x00,                  /* wcChar11 */
    'O', 0x00,                  /* wcChar12 */
    'O', 0x00,                  /* wcChar13 */
    'L', 0x00,                  /* wcChar14 */
    'S', 0x00,                  /* wcChar15 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
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

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
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
            bootuf2_init();
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

void usbd_msc_get_cap(uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    *block_num = bootuf2_get_sector_count();
    *block_size = bootuf2_get_sector_size();

    USB_LOG_INFO("sector count:%d, sector size:%d.\r\n", *block_num, *block_size);
}
int usbd_msc_sector_read(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    boot2uf2_read_sector(sector, buffer, length / bootuf2_get_sector_size() );
    return 0;
}

int usbd_msc_sector_write(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    //USB_LOG_INFO("usbd_msc_sector_write : sector = 0x%08X , length = %d.\r\n" , sector , length );
    bootuf2_write_sector(sector, buffer, length / bootuf2_get_sector_size() );
    return 0;
}

static struct usbd_interface intf0;

void msc_bootuf2_init(uint8_t busid, uintptr_t reg_base)
{
    boot2uf2_flash_init();
#ifdef CONFIG_USBDEV_ADVANCE_DESC
    usbd_desc_register(busid, &msc_bootuf2_descriptor);
#else
    usbd_desc_register(busid, msc_bootuf2_descriptor);
#endif

    usbd_add_interface(busid, usbd_msc_init_intf(busid, &intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_initialize(busid, reg_base, usbd_event_handler);
}

void boot2uf2_flash_init(void)
{

}

int bootuf2_flash_write(uint32_t address , const uint8_t *data, size_t size)
{
    uint32_t page_count = 0 ;

    address = address + FLASH_BASE ;

    FLASH_Unlock_Fast();

    USB_LOG_INFO("address:0x%08X, size:%d , addr=0x%08X.\r\n", address , size  , data );

    for ( page_count = 0 ; page_count < size ; ) 
    {
        //Erase 256Byte
        FLASH_ErasePage_Fast( address + page_count );
        //Programage 256Byte
        FLASH_ProgramPage_Fast( address + page_count , (uint32_t *)( data + page_count ) );
        page_count += 256 ;
    }

    FLASH_Lock_Fast(); 
    
    return 0;
}

