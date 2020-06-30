/**
 * @desc: 步进电机驱动控制程序
 */

#include "stepper.h"

#define LOG_TAG             "stepper"
#include <drv_log.h>
#include <stdlib.h>


static rt_uint8_t forward_table[4] = {0x09,0x0A,0x06,0x05};  //4拍正转表 Forward
static rt_uint8_t reverse_table[4] = {0x05,0x06,0x0A,0x09};  //4拍反转表 Reverse



/* ZOOM步进电机初始定义 名称、引脚等 */
stepper_t zoomStepper = { // 拉远拉近
	.name   = "zoom",

	.Ap_pin = GET_PIN(B, 3), /* A+ */
	.Am_pin = GET_PIN(B, 4), /* A- */
	.Bp_pin = GET_PIN(B, 5), /* B+ */
	.Bm_pin = GET_PIN(B, 6), /* B- */
};

/* FOCUS步进电机初始定义 名称、引脚等 */
stepper_t focusStepper = { // 放大缩小
	.name   = "focus",

	.Ap_pin = GET_PIN(B, 7),  /* A+ */
	.Am_pin = GET_PIN(B, 8),  /* A- */
	.Bp_pin = GET_PIN(B, 9),  /* B+ */
	.Bm_pin = GET_PIN(B, 10), /* B- */
};

/**
 * @brief  步进电机失停止失能
 * @param  *stepper 步进电机结构体指针
 */
void stepper_stop(stepper_t *stepper)
{
	rt_pin_write(stepper->Ap_pin, PIN_LOW);
	rt_pin_write(stepper->Am_pin, PIN_LOW);
	rt_pin_write(stepper->Bp_pin, PIN_LOW);
	rt_pin_write(stepper->Bm_pin, PIN_LOW);
}

/**
 * @brief  步进电机引脚初始化
 * @param  *stepper 步进电机结构体指针
 */
void stepper_init(stepper_t *stepper)
{
	rt_pin_mode(stepper->Ap_pin, PIN_MODE_OUTPUT);
	rt_pin_mode(stepper->Am_pin, PIN_MODE_OUTPUT);
	rt_pin_mode(stepper->Bp_pin, PIN_MODE_OUTPUT);
	rt_pin_mode(stepper->Bm_pin, PIN_MODE_OUTPUT);

	/* 引脚初始化后，拉低停止，确保稳定 */
	stepper_stop(stepper);
}

/**
 * @brief  根据节拍控制字选择输出引脚
 * @param  *stepper 步进电机结构体指针，beat 节拍控制字
 */
void selete_pin(rt_uint32_t pin, rt_uint8_t beat)
{
	if(beat)
		rt_pin_write(pin, PIN_HIGH);
	else
		rt_pin_write(pin, PIN_LOW);
}

/**
 * @brief  步进电机控制
 * @param  *stepper 步进电机结构体指针，*beat_table 节拍控制表首地址，*angle角度变量指针
 */
void stepper_set(stepper_t *stepper, rt_uint8_t *beat_table, rt_int16_t *angle)
{

	/* 角度自减，等待完成对应的角度数 */
	while(((*angle)--) > 0) // 由于数据手册未给定步距角，暂定为1步进1°
	{
		for(int k = 0; k < 4; k++) // 4节拍，完成一个步进
		{
			selete_pin(stepper->Ap_pin,beat_table[k] & 0x08); /* A+ */
			selete_pin(stepper->Am_pin,beat_table[k] & 0x04); /* A- */
			selete_pin(stepper->Bp_pin,beat_table[k] & 0x02); /* B+ */
			selete_pin(stepper->Bm_pin,beat_table[k] & 0x01); /* B- */
			rt_thread_mdelay(1); // 周波数大约为1000pps，因此这里延时1ms
		}
	}
	stepper_stop(stepper); // 步进电机停转
}

/**
 * @brief  步进电机控制
 * @param  *stepper 步进电机结构体指针，*dir 正反转标志指针，*angle角度变量指针(0~360)
 */
void stepper_control(stepper_t *stepper, stepper_dir_e *dir, rt_int16_t *angle)
{
	
	if(ANTICLOCKWISE == *dir)   // 正转
		stepper_set(stepper,forward_table,angle);
	
	else if(CLOCKWISE == *dir)  // 反转
		stepper_set(stepper,reverse_table,angle);
	
	*dir = STOP;
}

/**
 * @brief  步进电机 msh调试命令
 * @param  stepper_set <dir> <angle>
 */
static int stepper(int argc, char *argv[])
{
	int result = 0;
	stepper_dir_e stepper_dir;
	rt_int16_t    stepper_angle[2];

	if(argc != 3)
	{
		LOG_E("params error! Usage: stepper_set <dir> <angle>\r\n eg. stepper_set 1 120");
		result = -RT_ERROR;
		goto exit;
	}

	stepper_dir      = (stepper_dir_e)atoi(argv[1]); // 获取 方向标志
	stepper_angle[0] = atoi(argv[2]);    // 获取 角度数值
	stepper_angle[1] = stepper_angle[0]; // 角度由于会自减，因此两个步进电机需要两个变量
	
	if(stepper_angle[0] < 0 || stepper_angle[0] > 360)
	{
		LOG_E("angle range in (0,360)");
		result = -RT_ERROR;
		goto exit;
	}
	
	stepper_control(&zoomStepper , &stepper_dir,&stepper_angle[0]);
	stepper_control(&focusStepper, &stepper_dir,&stepper_angle[1]);
	
exit:
	return result;

}
MSH_CMD_EXPORT(stepper, stepper_set <dir> <angle>);


