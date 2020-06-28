/**
 * @desc: A4988为步进电机驱动控制程序
 */

#include "pwm.h"

#define LOG_TAG             "pwm"
#include <drv_log.h>

#include <stdlib.h>
/* defined the Stepper Dir pin: B3 B6 */
#define FOCUS_DIR_PIN    GET_PIN(B, 3)
#define ZOOM_DIR_PIN     GET_PIN(B, 6)

#define PWM_DEV_NAME        "pwm3"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL1     1       /* PWM通道 */
#define PWM_DEV_CHANNEL2     2       /* PWM通道 */
static struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */

/* 步进方向 枚举  0逆 1正*/
typedef enum 
{
		ANTICLOCKWISE, // 逆时针 
		CLOCKWISE,     // 顺时针
}stepperDir_e;

/* 步进开启关闭 枚举 0关 1开*/
typedef enum 
{
		STEPPER_OFF, // 关闭
		STEPPER_ON , // 开启
}stepperOnOff_e;

/* 步进电机 结构体*/
typedef struct 
{	
		char *name;					 /* 步进电机名称 */
		char *pwm_dev_name;  /* PWM设备名称 */
	 	rt_uint8_t    channel; /* PWM通道 */
		rt_uint32_t   period;  /* 周期，单位为纳秒ns */
		rt_uint32_t   pulse;   /* 占空比，单位为纳秒ns */
		rt_uint32_t   dir_pin; /* 方向引脚 */

}stepper_t;

/* 初始定义 PWM设备、通道、引脚等 */
stepper_t focusStepper = { // PWM CH1/B3
		.name         = "focus",
		.pwm_dev_name = PWM_DEV_NAME,
		.channel      = PWM_DEV_CHANNEL1,
		.period       = 2000000,   /* 周期为2ms，单位为纳秒ns */
		.pulse        = 1000000,   /* 占空比1ms，单位为纳秒ns */
		.dir_pin      = FOCUS_DIR_PIN,

};

stepper_t zoomStepper = { // PWM CH2/B6
		.name         = "zoom",
		.pwm_dev_name = PWM_DEV_NAME,
		.channel      = PWM_DEV_CHANNEL2,
		.period       = 2000000,   /* 周期为2ms，单位为纳秒ns */
		.pulse        = 1000000,   /* 占空比1ms，单位为纳秒ns */
		.dir_pin      = ZOOM_DIR_PIN,
};



void stepper_control(stepper_t *stepper,stepperOnOff_e onoff,stepperDir_e dir)
{
		if(ANTICLOCKWISE == dir) 
				rt_pin_write(stepper->dir_pin, PIN_HIGH); // 逆时针0
		else if(CLOCKWISE == dir)
				rt_pin_write(stepper->dir_pin, PIN_LOW);	// 顺时针1
		else // stepperDir param error"
				return ;
				


	
		if(STEPPER_ON == onoff)
				rt_pwm_set(pwm_dev, stepper->channel, stepper->period, stepper->pulse); // 开启脉冲
		else if(STEPPER_OFF == onoff)
		{
				rt_pwm_set(pwm_dev, stepper->channel, stepper->period, 0); // 关闭脉冲
				return; // 关闭则直接返回
		}
		else // stepperOnOff param error
				return;
		

		
}
stepperOnOff_e msh_onoff;
stepperDir_e   msh_dir;

static int stepper_set(int argc, char *argv[])
{
		if(argc != 3)
		{
				LOG_E("Params error! Usage: stepper_set <on-off> <dir> \n\
							 eg. stepper_set 1 1 (on-clockwise)");
				return -RT_ERROR;
		}
		msh_onoff = (stepperOnOff_e)atoi(argv[1]);
		msh_dir   = (stepperDir_e)  atoi(argv[2]);
		return 0;
		
}
MSH_CMD_EXPORT(stepper_set, eg: stepper_set 1 1);


/* 线程入口 */
static void thread_entry(void* parameter)
{
    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (RT_NULL == pwm_dev)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return;
    }
		else
				LOG_I("pwm3 init success");

    /* 使能设备 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL1);
		rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL2);

    while (1)
    {
				stepper_control(&focusStepper,msh_onoff, msh_dir);
				stepper_control(&zoomStepper ,msh_onoff, msh_dir);
				rt_thread_mdelay(10);
    }
}

static int pwm_init(void)
{
    rt_thread_t tid = RT_NULL;
    /* 创建线程 1 */
    tid = rt_thread_create("stepper",
                            thread_entry, RT_NULL,
                            1024,
                            10, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    rt_pin_mode(focusStepper.dir_pin, PIN_MODE_OUTPUT);
	  rt_pin_mode(zoomStepper.dir_pin , PIN_MODE_OUTPUT);
	
		rt_pin_write(focusStepper.dir_pin, PIN_HIGH);
		rt_pin_write(zoomStepper.dir_pin , PIN_HIGH);
    return 0;
}
INIT_APP_EXPORT(pwm_init);
