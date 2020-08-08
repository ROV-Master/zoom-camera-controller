#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <rtthread.h>
#include <board.h>

/* 初始化时，每次接受到指令的 固定的步进角 */
#define STEP_ANGLE  10

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
	rt_uint32_t   Ap_pin; /* A+ */
	rt_uint32_t   Am_pin; /* A- */
	rt_uint32_t   Bp_pin; /* B+ */
	rt_uint32_t   Bm_pin; /* B- */
	stepper_dir_e dir;    /* 方向 */
	rt_int16_t    angle;  /* 角度 */
	rt_int16_t    fix_angle; /* 每接收到指令转动的固定角度 */
}stepper_t;

extern stepper_t zoomStepper,focusStepper;

/* 步进电机引脚初始化 */
void stepper_init(stepper_t *stepper);
/* 步进电机控制 */
void stepper_control(stepper_t *stepper);
/* 设定步进电机参数 */
void set_stepper_params(stepper_t *stepper, uint8_t data);
#endif
