#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_
#include "lcd.h"
#include "sensor.h"
#include "internet.h"

void lcd_int(void);
void display_sensor_data(sensors_t* sensor_data);
void display_metar(report_metar_t* metar_data);
void display_taf(report_taf_t* taf_data_q);
void console_display(UART_HandleTypeDef *huart, char* text, uint32_t size);

#endif /* INC_DISPLAY_H_ */
