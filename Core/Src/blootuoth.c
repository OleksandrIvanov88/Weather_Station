/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */
#include "blootuoth.h"
#include "usart.h"
#include "BMP280.h"
#include "globaldata.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

static void bt_display_bmp(char* cmd2, uint8_t* tx_buf);
static void bt_diplay_metar (uint8_t *tx_buf);
static void bt_diplay_taf (uint8_t *tx_buf);

/**
 *  Provides ring buffer initialization fro blootuoth.
 *  @context: should be used only one time before parcin_bt_command().
 */
void blootuoth_int(void)
{
	uint8_t rx_buf_bt[BUF_SIZE_BT];
	/* Init RingBuffer_DMA object */
	RingBuffer_DMA_Init(&rx_bt, huart5.hdmarx, rx_buf_bt, sizeof(rx_buf_bt) / sizeof(rx_buf_bt[0]));
	/* Start UART4 DMA Reception */
	HAL_UART_Receive_DMA(&huart5, rx_buf_bt, sizeof(rx_buf_bt) / sizeof(rx_buf_bt[0]));
}

/**
 *  Provides blootuoth commands parcing
 */
void parcin_bt_command(void)
{
	/* Check number of bytes in RingBuffer */
	uint32_t rx_count = RingBuffer_DMA_Count(&rx_bt);
	/* Process each byte individually */
	while (rx_count--) {
		char ch = (char) RingBuffer_DMA_GetByte(&rx_bt);
		switch (ch) {
		case '\r':
			continue;
		case '\n':
			cmd_bt[cmd_i] = '\0';
			cmd_i = 0;
			comand_handling(cmd_bt);
			break;
		default:
			cmd_bt[cmd_i++] = ch;
			if (cmd_i >= sizeof(cmd_bt) / sizeof(cmd_bt[0])) {
				cmd_i = 0;
			}
			break;
		}
	}
}

/**
 *  Provides processing of bluetooth commands
 *  @input: command for processing
 */
void comand_handling(char *input)
{
	extern osMutexId_t mutex_sensorsHandle;
	extern osMutexId_t mutex_metarHandle;
	extern osMutexId_t mutex_tafHandle;
	uint8_t tx_buf[BUF_SIZE_BT];
	char cmd_1[CMD_SIZE / 2] = { 0 };
	char cmd_2[CMD_SIZE / 2] = { 0 };
	uint8_t cmd_amount = sscanf(input, "%s %63s", cmd_1, cmd_2);
	if (cmd_amount == 2) {
		if (!strcmp(cmd_1, "bmp280")) {
			if (osMutexAcquire(mutex_sensorsHandle,osWaitForever) == osOK) {
				bt_display_bmp(cmd_2, tx_buf);
				osMutexRelease(mutex_sensorsHandle);
			}
		} else {
			snprintf((char*) tx_buf, BUF_SIZE_BT, "Unknown command\r\n");
		}
	} else if (!strcmp(cmd_1, "metar") || !strcmp(cmd_1, "Metar") || !strcmp(cmd_1, "METAR")){

		if(osMutexAcquire( mutex_metarHandle, osWaitForever) == osOK) {
			bt_diplay_metar(tx_buf);
			osMutexRelease( mutex_metarHandle);
		}
	} else if(!strcmp(cmd_1, "taf") || !strcmp(cmd_1, "Taf") || !strcmp(cmd_1, "TAF")){
		if (osMutexAcquire(mutex_tafHandle, osWaitForever) == osOK) {
			bt_diplay_taf(tx_buf);
			osMutexRelease(mutex_tafHandle);
		}
	} else {
		snprintf((char*) tx_buf, BUF_SIZE_BT, "Unknown command\r\n");
	}
	HAL_UART_Transmit(&huart5, tx_buf, strlen((char*) tx_buf), 1000);
}

/**
 *  Processing bluetooth command's sensor argument and save sensors data to tx_buf
 *  @cmd_2: sensors' command argument
 *  @tx_buf: buffer for saving sensors data
 */
void bt_display_bmp(char* cmd_2, uint8_t* tx_buf)
{
	if (!strcmp(cmd_2, "temp")) {
		snprintf((char*) tx_buf, BUF_SIZE_BT, "Temperature: %+5.2lf C\r\n", sensor_data_bt.temp);
	} else if (!strcmp(cmd_2, "press")) {
		snprintf((char*) tx_buf, BUF_SIZE_BT, "Pressure: %6.3lf kPa\r\n", sensor_data_bt.press / 1000.0);
	} else if (!strcmp(cmd_2, "alt")) {
		(QNH_global == QNH_GOT_CHECK) ? (snprintf((char*) tx_buf, BUF_SIZE_BT, "Altitude: waiting QNH\r\n"))
				         : snprintf((char*) tx_buf, BUF_SIZE_BT, "Alt  : %+5.1lf m\r\n", sensor_data_bt.alt);
	} else if (!strcmp(cmd_2, "all")) {
		if (QNH_global == QNH_GOT_CHECK) {
			snprintf((char*) tx_buf, BUF_SIZE_BT, "Temperature : %+5.2lf C\r\nPressure: %6.3lf kPa\r\nAlt : waiting QNH\r\n",
					                                           sensor_data_bt.temp, sensor_data_bt.press / 1000.0);
		} else {
			snprintf((char*) tx_buf, BUF_SIZE_BT, "Temperature : %+5.2lf C\r\nPressure: %6.3lf kPa\r\nAltitude : %+5.1lf m\r\n",
					                     sensor_data_bt.temp, sensor_data_bt.press / 1000.0, sensor_data_bt.alt);
		}
	} else {
		snprintf((char*) tx_buf, BUF_SIZE_BT, "Unknown command argument\r\n");
	}
}

/**
 *  Saves METAR report data to tx_buf
 *  @tx_buf: buffer for saving data
 */
void bt_diplay_metar (uint8_t *tx_buf)
{
	snprintf((char*) tx_buf, BUF_SIZE_BT,"%s\r\nTemperature = %d C\r\nDewpoint = %d C\r\n"
						"Pressure = %d hPa\r\nWind dir = %d degree\r\nWind speed = %d m/s2\r\n"
						"Visibility = %d m\r\n", metar_data_bt.timestamp_m_report
												,metar_data_bt.temp_m_report
												,metar_data_bt.dewpoint_m_report
												,metar_data_bt.QNH_m_report
												,metar_data_bt.wind_direction_m_report
												,metar_data_bt.wind_speed_m_report
												,metar_data_bt.visibility_m_report);
}

/**
 *  Saves TAF report data to tx_buf
 *  @tx_buf: buffer for saving data
 */
void bt_diplay_taf (uint8_t *tx_buf)
{
	snprintf((char*) tx_buf, BUF_SIZE_BT,"%s\r\nTime issue = %s\r\nWind dir = %d degree\r\nWind speed = %d m/s2\r\n"
									"Visibility = %d m\r\n" ,taf_data_bt.timestamp_t_report
															,taf_data_bt.timeissue_t_report
															,taf_data_bt.wind_direction_t_report
															,taf_data_bt.wind_speed_t_report
															,taf_data_bt.visibility_t_report);
}
