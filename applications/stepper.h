#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <rtthread.h>
#include <board.h>

/* 步进方向 枚举  0逆 1正*/
typedef enum 
{
	STOP          = 0, // 停止
	ANTICLOCKWISE = 1, // 逆时针 正转 
	CLOCKWISE     = 2, // 顺时针 反转
}stepper_dir_e;



/* 步进电机 结构体*/
typedef struct 
{	
	char *name;					 // 步进电机名称 
	
	/* 步进电机引脚定义*/
	rt_uint32_t  Ap_pin; /* A+ */
	rt_uint32_t  Am_pin; /* A- */
	rt_uint32_t  Bp_pin; /* B+ */
	rt_uint32_t  Bm_pin; /* B- */
	
}stepper_t;

extern stepper_t zoomStepper,focusStepper;

/* 步进电机引脚初始化 */
void stepper_init(stepper_t *stepper);
/* 步进电机控制 */
void stepper_control(stepper_t *stepper, stepper_dir_e *dir, rt_int16_t *angle);
#endif
