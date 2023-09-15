/* Includes ------------------------------------------------------------------*/
#include "usb_utils.h"
#include "main.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char usb_tx_buffer[USB_BUF_SIZE] = { 0 };
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief
 * @note 放在下同初始化之后，外设初始化之前
 */
void usb_reset(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
}

inline void usb_transmit(uint8_t *data, uint16_t len)
{
	CDC_Transmit_FS(data, len);
}

void usb_printf(const char *format, ...)
{
	int len;
	va_list args;
	va_start(args, format);
	len = vsnprintf(usb_tx_buffer, USB_BUF_SIZE, format, args);
	va_end(args);

	usb_transmit((uint8_t *)usb_tx_buffer, len);
}
