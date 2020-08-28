/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */
#include "sensor.h"
#include "lcd.h"
#include "BMP280.h"
#include "internet.h"
#include "globaldata.h"
#include "display.h"
#include "usart.h"
#include "debug.h"
#include <string.h>


/**
 *  Scans I2C line for the devices address
 *  @i2c_addr: array for seving diveces' address.
 *  @hi2cx: I2C handle.
 *  @device_amount: connected device amount.
 *  @return: 1 if one or more devices found and 0 if no device found.
 */
uint8_t i2c_address_scan(uint8_t* i2c_addr, I2C_HandleTypeDef* hi2cx, uint8_t* device_amount)
{
	*device_amount = 0;

	for(size_t i =  0x08; i <  0x78; ++i)
	{
		if(HAL_I2C_IsDeviceReady(hi2cx, i << 1, 1, 100) == HAL_OK)
		{
			i2c_addr[(*device_amount)++] = i;
		}
	}
	if(*device_amount > 0)
	{
		return 1;
	}
	return 0;
}


/**
 *  Start BMP 280 sensors
 *  @return: 1 if success 0 if not.
 */
uint8_t bmp_start(void)
{
	QNH_global = QNH_GOT_CHECK;
	/* Start BMP280 and change settings */
	int8_t com_rslt;
	static bmp280_t bmp280;
	bmp280.i2c_handle = &hi2c1;
	bmp280.dev_addr = BMP280_I2C_ADDRESS1;
	com_rslt = BMP280_init(&bmp280);
	com_rslt += BMP280_set_power_mode(BMP280_NORMAL_MODE);
	com_rslt += BMP280_set_work_mode(BMP280_STANDARD_RESOLUTION_MODE);
	com_rslt += BMP280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);
	if (com_rslt != SUCCESS) {
		return 1;
	}
	return 0;
}

/**
 *  Provides receipt of sensor data and writing them to the structure.
 *  @sensor_data: structure for writing data.
 */
void sensor_get_data(sensors_t* sensor_data)
{
	BMP280_read_temperature_double(&sensor_data->temp);
	BMP280_read_pressure_double(&sensor_data->press);
	/* Calculate current altitude, based on current QNH pressure */
	if(QNH_global != QNH_GOT_CHECK) {
		sensor_data->alt = BMP280_calculate_altitude(QNH_global * 100);
	}
}


/**
 *  Start BMP 280 sensors initialization
 *  @return: 1 if success 0 if not.
 */
uint8_t sensors_init(void)
{
	uint8_t i2c_addr[I2C_MAX_ADDR] = { 0 };
	uint8_t device_amount = 0;

	if (!i2c_address_scan(i2c_addr, &hi2c1, &device_amount)) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!!i2c_address_scan().*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		return 0;
	}

	if (bmp_start()) {
#ifdef CONSOLE_DEBUG_ERROR
		char *error = "*****Error!!!!bmp_start().*****\r\n";
		console_display(&huart2, error, strlen(error));
#endif
		return 0;
	}
	return 1;
}

/**
 * Write sensors' data to structure for bluetooth thread
 *  @sensor_data: sensors data
 */
void write_sensor_data_bt(sensors_t* sensor_data)
{
	memcpy(&sensor_data_bt, sensor_data, sizeof (sensor_data_bt));
}

