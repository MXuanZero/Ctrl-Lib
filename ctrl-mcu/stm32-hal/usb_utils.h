#ifndef USB_UTILS_H
#define USB_UTILS_H
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Define --------------------------------------------------------------------*/
#define USB_BUF_SIZE 128
/* Exported macro ------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void usb_reset(void);
void usb_transmit(uint8_t *data, uint16_t len);
void usb_printf(const char *format, ...);
#endif // USB_UTILS_H
