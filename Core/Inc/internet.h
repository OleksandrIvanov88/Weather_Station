#ifndef INC_INTERNET_H_
#define INC_INTERNET_H_

#include "ringbuffer_dma.h"
#include <stdio.h>
#include "globaldata.h"

#define BUF_SIZE_WIFI 8192

/* ESP8266 variables */

#define SERVER_ADDRESS		"avwx.rest"
#define SERVER_TOKEN		"**********************************"
#define WIFI_SSID			"*************"
#define WIFI_PASS			"*************"
#define AIRPORT			    "UKKK"

#define ERROR_VALUE         -999999

/* Ringbuffer for Rx messages */
RingBuffer_DMA rx_buf_wifi;
/* Array for DMA to save Rx bytes */
uint8_t rx_wifi[BUF_SIZE_WIFI];
/* Array for received commands */
char wifi_response[BUF_SIZE_WIFI];
char wifi_cmd[512];

typedef struct {
	int QNH_m_report;
	int temp_m_report;
	int dewpoint_m_report;
	int visibility_m_report;
	int wind_direction_m_report;
	int wind_speed_m_report;
	char timestamp_m_report[32];
}report_metar_t;

typedef struct {
	char timestamp_t_report[32];
	char timeissue_t_report[32];
	int visibility_t_report;
	int wind_direction_t_report;
	int wind_speed_t_report;
} report_taf_t;

void wifi_init(void);
void htpp_request_metar(void);
void htpp_request_taf(void);
void write_metar_data_bt(report_metar_t* metar_data);
void write_taf_data_bt(report_taf_t* metar_data);


#endif /* INC_INTERNET_H_ */
