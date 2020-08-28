#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_

#include <stdint.h>
#include "i2c.h"

#define I2C_MAX_ADDR    112

typedef struct {
	double temp;
	double press;
	double alt;
}sensors_t;

uint8_t sensors_init(void);
uint8_t i2c_address_scan(uint8_t* i2c_addr, I2C_HandleTypeDef* hi2cx, uint8_t* device_amount);
void i2c_print_address(uint8_t* list, uint8_t device_amount);
uint8_t bmp_start(void);
void sensor_get_data(sensors_t* sensor_data);
void write_sensor_data_bt(sensors_t* sensor_data);


#endif /* INC_SENSOR_H_ */
