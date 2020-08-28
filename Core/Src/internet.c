/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */
#include "internet.h"
#include "usart.h"
#include "lcd.h"
#include "display.h"
#include "cJSON.h"
#include "cmsis_os.h"
#include "debug.h"
#include <string.h>


#ifdef CONSOLE_DEBUG_AT
static void UART_Read_AT(void);
#endif

static void UART_Read_http(void);
static uint8_t json_parsing_metar(const char* const data);
static uint8_t json_parsing_taf(const char* const data);
static uint8_t json_get_data_int(const cJSON* const root, char* object_name, int *data, char* param);
static uint8_t json_get_data_str(const cJSON* const root, char* object_name, char *data, char* param);
static void parcing_data_metar(void);
static void get_clean_json(char* clean_json);
static void parcing_data_taf(void);


/**
 *  Provides internet connection and dma start
 *  @context: should be used only one time before http reques.
 */
void wifi_init(void)
{
	/* Init RingBuffer_DMA object */
	RingBuffer_DMA_Init(&rx_buf_wifi, huart4.hdmarx, rx_wifi, BUF_SIZE_WIFI);
	/* Start UART4 DMA Reception */
	HAL_UART_Receive_DMA(&huart4, rx_wifi, BUF_SIZE_WIFI);

#ifdef CONSOLE_DEBUG_AT
	/* CONNECT TO WIFI ROUTER */
	/* Simple ping */
	sprintf(wifi_cmd, "AT+RST\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(500);
	UART_Read_AT();

	/* Turn on message echo */
	sprintf(wifi_cmd, "ATE1\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(5);
	UART_Read_AT();

	/* Display version info */
	sprintf(wifi_cmd, "AT+GMR\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(30);
	UART_Read_AT();
#endif

	/* Set to client mode */
	sprintf(wifi_cmd, "AT+CWMODE_CUR=1\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(10);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
	/* Connect to network */
	sprintf(wifi_cmd, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(10000);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
	/* CONNECTED (hope so) */
#ifdef CONSOLE_DEBUG_AT
	/* Check for IP */
	sprintf(wifi_cmd, "AT+CIFSR\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(30);

	UART_Read_AT();
#endif
}

/**
 *  Executes http request for METAR.
 */
void htpp_request_metar(void) {
	/* Create request string for METAR*/
	char http_req[256];
	sprintf(http_req, "GET /api/metar/%s?token=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			           AIRPORT,SERVER_TOKEN,SERVER_ADDRESS);

	/* Connect to server */
	sprintf(wifi_cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", SERVER_ADDRESS);
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(500);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
	/* Send data length (length of request) */
	sprintf(wifi_cmd, "AT+CIPSEND=%d\r\n", strlen(http_req));
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(20); // wait for ">"
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
	/* Send data METAR*/
	HAL_UART_Transmit(&huart4, (uint8_t*) http_req, strlen(http_req), 1000);
	osDelay(2000);
	parcing_data_metar();

	/* Disconnect from server */
	sprintf(wifi_cmd, "AT+CIPCLOSE\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(20);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
}

/**
 *  Executes http request for TAF.
 */
void htpp_request_taf(void)
{
	char http_req[256];
	/* Create request string for TAF*/
	sprintf(http_req, "GET /api/taf/%s?token=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			AIRPORT, SERVER_TOKEN, SERVER_ADDRESS);

	/* Connect to server */
	sprintf(wifi_cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", SERVER_ADDRESS);
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(500);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif

	/* Send data length (length of request) */
	sprintf(wifi_cmd, "AT+CIPSEND=%d\r\n", strlen(http_req));
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(20); // wait for ">"
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif

	/* Send data TAF*/
	HAL_UART_Transmit(&huart4, (uint8_t*) http_req, strlen(http_req), 1000);
	osDelay(2500);
	parcing_data_taf();

	/* Disconnect from server */
	sprintf(wifi_cmd, "AT+CIPCLOSE\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*) wifi_cmd, strlen(wifi_cmd), 1000);
	osDelay(20);
#ifdef CONSOLE_DEBUG_AT
	UART_Read_AT();
#endif
}


/**
 *  Reads AT's commands response of wifi module from the ring buffer and displays them to console.
 *  @context: required only for debug
 */
#ifdef CONSOLE_DEBUG_AT
static void UART_Read_AT(void)
{
	/* Check number of bytes in RingBuffer */
    int32_t rx_count_wifi = RingBuffer_DMA_Count(&rx_buf_wifi);
	uint32_t icmd = 0;
	console_display(&huart2, "\r\n", strlen("\r\n"));
	while (rx_count_wifi--) {
		/* Read out one byte from RingBuffer */
		uint8_t b = RingBuffer_DMA_GetByte(&rx_buf_wifi);
		if (b == '\n') { /* If \n process command */
			/* Terminate string with \0 */
			wifi_response[icmd] = 0;
			icmd = 0;
			/*transmite to console*/
			console_display(&huart2, wifi_response, strlen(wifi_response));
			console_display(&huart2, "\r\n", strlen("\r\n"));

		} else if (b == '\r') { /* If \r skip */
			continue;
		} else { /* If regular character, put it into wifi_response[] */
			wifi_response[icmd++ % BUF_SIZE_WIFI] = b;
		}
	}
}
#endif

/**
 *  Reads http response from the server and display them to console in debug mode
 */
static void UART_Read_http(void)
{
	/* Check number of bytes in RingBuffer */
	int32_t rx_count_wifi = RingBuffer_DMA_Count(&rx_buf_wifi);
	uint32_t icmd = 0;
	wifi_response[icmd] = 0;
	while (rx_count_wifi--) {
		uint8_t b = RingBuffer_DMA_GetByte(&rx_buf_wifi);
		wifi_response[icmd++ % BUF_SIZE_WIFI] = b;
	}
#ifdef CONSOLE_DEBUG_AT
	console_display(&huart2, wifi_response, strlen(wifi_response));
	console_display(&huart2, "\r\n", strlen("\r\n"));
#endif
}

/**
 *  Provides JSON parcing of METAR report
 *  @data: data for parsing
 *  @return: 1 in success or 0 if not
 */
static uint8_t json_parsing_metar(const char* const data)
{
	int status = 1;
	cJSON *metar_root = cJSON_Parse(data);
	if (metar_root == NULL) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_Parse() - metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	report_metar_t metar_data_q;
	if(!json_get_data_int(metar_root, "altimeter", &metar_data_q.QNH_m_report, "value")) {
		metar_data_q.QNH_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - QNH metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	if(!json_get_data_int(metar_root, "temperature", &metar_data_q.temp_m_report, "value")) {
		metar_data_q.temp_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - temp metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	if(!json_get_data_int(metar_root, "dewpoint", &metar_data_q.dewpoint_m_report, "value")) {
		metar_data_q.dewpoint_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - dewpoint metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	if(!json_get_data_int(metar_root, "visibility", &metar_data_q.visibility_m_report, "value")) {
		metar_data_q.visibility_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - visibility metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	if(!json_get_data_int(metar_root, "wind_direction", &metar_data_q.wind_direction_m_report, "value")) {
		metar_data_q.wind_direction_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - wind_direction metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	if(!json_get_data_int(metar_root, "wind_speed", &metar_data_q.wind_speed_m_report, "value")) {
		metar_data_q.wind_speed_m_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - wind_speed metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	char timestamp[32] = {0};
	if(!json_get_data_str(metar_root, "meta", timestamp, "timestamp")) {
#ifdef CONSOLE_DEBUG_ERROR
		char* error = "*****Error!!!json_get_data_str() - timestamp metar*****.\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	QNH_global = metar_data_q.QNH_m_report;
	strcpy(metar_data_q.timestamp_m_report, timestamp);

	extern osMessageQueueId_t queue_reportMetarHandle;
	if (osMessageQueuePut(queue_reportMetarHandle, &metar_data_q, 0, 9) != osOK) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!osMessageQueuePut() - metar.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
	}
	cJSON_Delete(metar_root);
	return status;
}


/**
 *  Extract integer value from cJSON structure
 *  @root: cJSON structure
 *  @object_name: nema of required object
 *  @data: variable for saving extracted data
 *  @param: parameter name
 *  @return: 1 in success or 0 if not
 */
static uint8_t json_get_data_int(const cJSON* const root, char* object_name, int *data, char* param)
{
	const cJSON *object = cJSON_GetObjectItemCaseSensitive(root, object_name);
	if (!cJSON_IsObject(object)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_IsObject().*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		return 0;
	}

	const cJSON *value = NULL;

	value = cJSON_GetObjectItemCaseSensitive(object, param);
	if (!cJSON_IsNumber(value)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_IsNumber().*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		return 0;
	}
	*data = value->valueint;

	return 1;
}

/**
 *  Extract string value from cJSON structure
 *  @root: cJSON structure
 *  @object_name: nema of required object
 *  @data: variable for saving extracted data
 *  @param: parameter name
 *  @return: 1 in success or 0 if not
 */
static uint8_t json_get_data_str(const cJSON* const root, char* object_name, char *data, char* param)
{
	const cJSON *object = cJSON_GetObjectItemCaseSensitive(root, object_name);
	if (!cJSON_IsObject(object)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_IsObject.*****";
		console_display(&huart2, error, strlen(error));
#endif
		return 0;
	}

	const cJSON *value = NULL;

	value = cJSON_GetObjectItemCaseSensitive(object, param);
	if (!(cJSON_IsString(value) && (value->valuestring != NULL))) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_IsString().*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
	}
	strcpy(data, value->valuestring);

	return 1;
}


/**
 *  Provides http parcing for METAR
 */
static void parcing_data_metar(void)
{
	UART_Read_http();
	char json_chunk[BUF_SIZE_WIFI] = { 0 };
	get_clean_json(json_chunk);

	if (!json_parsing_metar(json_chunk)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *erro = "*****Error!!!json_parsing_metar().*****\r\n";
		console_display(&huart2, erro, strlen(erro));
#endif
	}
}

/**
 *  Provides clean JSON format in case more than 1460 bytes are received from the server.
 *  When reading data over a wifi connection the esp8266 splits it into chunks of 1460 bytes maximum each.
 *  @clean_json: array to save clean json
 */
static void get_clean_json(char* clean_json)
{
	uint32_t json_cnt = 0;
	uint32_t icmd = 0;
	while (wifi_response[icmd++] != '{');
	clean_json[json_cnt++] = '{';

	while (!(wifi_response[icmd] == '}' && wifi_response[icmd + 1] == '}')) {
		if (wifi_response[icmd] == '\n' || wifi_response[icmd] == '\r') {
			icmd++;
			continue;
		} else if (wifi_response[icmd] == '+' && wifi_response[icmd + 1] == 'I'
				&& wifi_response[icmd + 2] == 'P'
				&& wifi_response[icmd + 3] == 'D') {
			while (wifi_response[icmd++] != ':')
				;
			continue;
		}
		clean_json[json_cnt++] = wifi_response[icmd++];
	}
	clean_json[json_cnt] = '}';
	clean_json[json_cnt + 1] = '}';
}


/**
 *  Provides http parcing for TAF
 */
static void parcing_data_taf(void)
{
	UART_Read_http();
	char json_chunk[BUF_SIZE_WIFI] = { 0 };
	get_clean_json(json_chunk);

	if (!json_parsing_taf(json_chunk)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *erro = "*****Error!!!json_parsing_taf().*****\r\n";
		console_display(&huart2, erro, strlen(erro));
#endif
	}
}

/**
 *  Provides JSON parcing of TAF report
 *  @data: data for parsing
 *  @return: 1 in success or 0 if not
 */
static uint8_t json_parsing_taf(const char* const data)
{
	int status = 1;
	cJSON *taf_root = cJSON_Parse(data);
	if (taf_root == NULL) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_Parse() - taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	char timestamp[32] = { 0 };
	if (!json_get_data_str(taf_root, "meta", timestamp, "timestamp")) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_str() - timestamp taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	char time_issue[32] = { 0 };
	if (!json_get_data_str(taf_root, "time", time_issue, "dt")) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_str() - timestamp taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	const cJSON *forecasts = cJSON_GetObjectItemCaseSensitive(taf_root, "forecast");
	if (!cJSON_IsArray(forecasts)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!cJSON_IsArray() - forecast.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	const cJSON *item = cJSON_GetArrayItem(forecasts, 0);
	report_taf_t taf_data_q;
	if (!json_get_data_int(item, "visibility", &taf_data_q.visibility_t_report, "value")) {
		taf_data_q.visibility_t_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - visibility taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	if (!json_get_data_int(item, "wind_direction", &taf_data_q.wind_direction_t_report, "value")) {
		taf_data_q.wind_direction_t_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - wind_direction taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}
	if (!json_get_data_int(item, "wind_speed", &taf_data_q.wind_speed_t_report, "value")) {
		taf_data_q.wind_speed_t_report = ERROR_VALUE;
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!json_get_data_int() - wind_speed taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		status = 0;
	}

	strcpy(taf_data_q.timestamp_t_report, timestamp);
	strcpy(taf_data_q.timeissue_t_report, time_issue);

	extern osMessageQueueId_t queue_reportTAFHandle;
	if (osMessageQueuePut(queue_reportTAFHandle, &taf_data_q, 0, 9) != osOK) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!osMessageQueuePut() - taf.*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
	}
	cJSON_Delete(taf_root);

	return status;
}


/**
 *  Write METAR parsed data to structure for bluetooth thread
 *  @metar_data: parced METAR's data
 */
void write_metar_data_bt(report_metar_t* metar_data)
{
	memcpy(&metar_data_bt, metar_data, sizeof (metar_data_bt));
}

/**
 * Write TAF parsed data to structure for bluetooth thread
 *  @taf_data: parced METAR's data
 */
void write_taf_data_bt(report_taf_t* taf_data)
{
	memcpy(&taf_data_bt, taf_data, sizeof (taf_data_bt));
}

