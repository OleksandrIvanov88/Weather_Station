/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */
#include "display.h"
#include "main.h"
#include "globaldata.h"
#include "cmsis_os.h"

/**
 *  Displays sensors' data on LCD.
 *  @sensor_data: Data to display.
 */
void display_sensor_data(sensors_t* sensor_data)
{
	LCD_SetCursor(0, 20);
	LCD_SetTextColor(WHITE, BLACK);
	LCD_Printf("BMP:\r\n");
	LCD_Printf("Temperature  = %.3lf C\r\n", sensor_data->temp);
	LCD_Printf("Pressure     = %.3lf kPa\r\n", sensor_data->press / 1000.0f);
	if(QNH_global == QNH_GOT_CHECK) {
		LCD_Printf("Altitude - waiting QNH\r\n");
	}else {
		LCD_Printf("Altitude     = %.2lf m\r\n", sensor_data->alt);
	}
}

/**
 *  Displays METAR report on LCD.
 *  @metar_data: Data to display.
 */
void display_metar(report_metar_t* metar_data)
{
	LCD_SetCursor(0, 80);
	LCD_Printf("METAR:\r\n");
	LCD_Printf("%s\n", metar_data->timestamp_m_report);
	(metar_data->temp_m_report == ERROR_VALUE) ? (LCD_Printf("Temperature NO DATA\r\n"))
			                : LCD_Printf("Temperature  = %d C\r\n", metar_data->temp_m_report);
	(metar_data->dewpoint_m_report == ERROR_VALUE) ? (LCD_Printf("Dewpoint No Data\r\n"))
			                : LCD_Printf("Dewpoint     = %d C\r\n", metar_data->dewpoint_m_report);
	(metar_data->QNH_m_report == ERROR_VALUE) ? (LCD_Printf("Pressure NO DATA\r\n"))
							: LCD_Printf("Pressure     = %d hPa\r\n", metar_data->QNH_m_report);
	(metar_data->wind_direction_m_report == ERROR_VALUE) ? (LCD_Printf("Wind dir NO DATA\r\n"))
							: LCD_Printf("Wind dir     = %d degree\r\n", metar_data->wind_direction_m_report);
	(metar_data->wind_speed_m_report == ERROR_VALUE) ? (LCD_Printf("Wind speed NO DATA\r\n"))
							: LCD_Printf("Wind speed   = %d m/s2\r\n", metar_data->wind_speed_m_report);
	(metar_data->visibility_m_report == ERROR_VALUE) ? (LCD_Printf("Visibility NO DATA\r\n"))
							: LCD_Printf("Visibility   = %d m\r\n", metar_data->visibility_m_report);
}

/**
 *  Displays TAF report on LCD.
 *  @taf_data: Data to display.
 */
void display_taf(report_taf_t* taf_data)
{
	LCD_SetCursor(0, 190);
	LCD_Printf("TAF:\r\n");
	LCD_Printf("%s\r\n", taf_data->timestamp_t_report);
	LCD_Printf("Time issue = %s\r\n", taf_data->timeissue_t_report);
	(taf_data->wind_direction_t_report == ERROR_VALUE) ? (LCD_Printf("Wind dir NO DATA\r\n"))
						   : LCD_Printf("Wind dir   = %d degree\r\n", taf_data->wind_direction_t_report);
	(taf_data->wind_speed_t_report == ERROR_VALUE) ? (LCD_Printf("Wind speed NO DATA\r\n"))
						   : LCD_Printf("Wind speed = %d m/s2\r\n", taf_data->wind_speed_t_report);
	(taf_data->visibility_t_report == ERROR_VALUE) ? (LCD_Printf("Visibility NO DATA\r\n"))
						   : LCD_Printf("Visibility = %d m \r\n", taf_data->visibility_t_report);
}

/**
 *  LCD initialization.
 */
void lcd_int(void)
{
	LCD_Init();
	LCD_FillScreen(BLACK);
}

/**
 *  Sends data to console.
 *  @context: is using for debug mode.
 *  @huart: UART handel.
 *  @text: string to display.
 *  @size: length of string.
 */
void console_display(UART_HandleTypeDef *huart, char* text, uint32_t size)
{
	extern osMutexId_t  mutex_consoleHandle;
	osMutexAcquire(mutex_consoleHandle, 0);
	HAL_UART_Transmit(huart, (uint8_t*) text , size, 1000);
	osMutexRelease(mutex_consoleHandle);
}
