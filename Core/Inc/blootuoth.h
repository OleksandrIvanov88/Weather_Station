/*
 * blootuoth.h
 *
 *  Created on: 3 авг. 2020 г.
 *      Author: User
 */

#ifndef INC_BLOOTUOTH_H_
#define INC_BLOOTUOTH_H_

#include "ringbuffer_dma.h"

#define BUF_SIZE_BT            1024
#define CMD_SIZE               128
#define QNH                    1014

RingBuffer_DMA rx_bt;
char cmd_bt[CMD_SIZE];
volatile uint32_t cmd_i;

void blootuoth_int(void);
void parcin_bt_command(void);
void comand_handling(char *input);

#endif /* INC_BLOOTUOTH_H_ */
