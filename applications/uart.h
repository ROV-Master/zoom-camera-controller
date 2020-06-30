#ifndef __MY_UART_H__
#define __MY_UART_H__

#include <rtthread.h>
#include <board.h>
#include "stepper.h"

/* 控制数据包长度 6 */
#define CONTROL_PACKET_LENGTH  6

/* 变焦控制命令 */
typedef struct
{
	stepper_dir_e zoom_dir;
	stepper_dir_e focus_dir;
	
	rt_int16_t zoom_angle;
	rt_int16_t focus_angle;
	
}cmd_t;

/* 控制数据解析 */
void control_data_analysis(rt_uint8_t data, cmd_t *cmd);

/* 控制数据 结构体 */
extern cmd_t cmd_data;

#endif
