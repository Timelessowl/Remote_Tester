#ifndef __SSD1306_IMAGES_H_
#define __SSD1306_IMAGES_H_

/*
 * Иконки для отрисовки на экране в формате битмап.
 * Единый формат для всех (4) надписей
 */

#define LABEL_MAX_LENGTH    24

// Иконка настройки(16x16)
#define CONFIG_IMG_WIDTH  (16)
#define CONFIG_IMG_HEIGHT (16)
#define CONFIG_IMG_ROWS   ((CONFIG_IMG_HEIGHT+7)/8)
const unsigned char CONFIG_IMG [] = {
		0x03, 0xC0, 0x12, 0x48, 0x2E, 0x74, 0x41, 0x82, 0x23, 0xC4, 0x24, 0x24, 0xE9, 0x97, 0x9B, 0xD9,
		0x9B, 0xD9, 0xE9, 0x97, 0x24, 0x24, 0x23, 0xC4, 0x41, 0x82, 0x2E, 0x74, 0x12, 0x48, 0x03, 0xC0
		};

// Иконка паузы(8x16)
#define PAUSE_IMG_WIDTH  (8)
#define PAUSE_IMG_HEIGHT (16)
#define PAUSE_IMG_ROWS   ((PAUSE_IMG_HEIGHT+7)/8)
const unsigned char PAUSE_IMG [] = {
		0x00, 0x00, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0x00, 0x00
		};

// Иконка ресета(16x16)
#define RESET_IMG_WIDTH  (16)
#define RESET_IMG_HEIGHT (16)
#define RESET_IMG_ROWS   ((RESET_IMG_HEIGHT+7)/8)
const unsigned char RESET_IMG [] = {
		0x00, 0x00, 0x00, 0x00, 0x0F, 0xC0, 0x1C, 0x70, 0x30, 0x38, 0x60, 0x18, 0x60, 0x08, 0x40, 0x0C,
		0x40, 0x3F, 0x60, 0x1E, 0x60, 0x0C, 0x30, 0x00, 0x1C, 0x70, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00
		};

// Иконка ресета(16x16)
#define ALERT_IMG_WIDTH  (16)
#define ALERT_IMG_HEIGHT (16)
#define ALERT_IMG_ROWS   ((ALERT_IMG_HEIGHT+7)/8)
const unsigned char ALERT_IMG [] = {
		0x00, 0x00, 0x01, 0x80, 0x03, 0xC0, 0x06, 0x60, 0x06, 0x60, 0x0D, 0xB0, 0x19, 0x98, 0x19, 0x98,
		0x39, 0x9C, 0x30, 0x0C, 0x61, 0x86, 0xE1, 0x87, 0xE0, 0x07, 0x7F, 0xFE, 0x3F, 0xFC, 0x00, 0x00
		};

typedef struct
{
	uint8_t start_x;
	uint8_t start_y;
	char label [LABEL_MAX_LENGTH];

}LABEL;


#endif
