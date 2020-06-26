/**
 * @desc: A4988为步进电机驱动控制程序
 */

#include "pwm.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LOG_TAG             "pwm"
#include <drv_log.h>


/* defined the Stepper Dir pin: B3 B6 */
#define DIR1_PIN    GET_PIN(B, 3)
#define DIR2_PIN    GET_PIN(B, 6)

#define PWM_DEV_NAME        "pwm3"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL1     1       /* PWM通道 */
#define PWM_DEV_CHANNEL2     2       /* PWM通道 */
static struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */


/* 线程入口 */
static void thread_entry(void* parameter)
{
	
	  rt_uint32_t period, pulse, dir;

    period = 20000000;    /* 周期为20ms，单位为纳秒ns */
    dir = 1;            /* PWM脉冲宽度值的增减方向 */
    pulse = 0;          /* PWM脉冲宽度值，单位为纳秒ns */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return;
    }
		else
		{
				LOG_I("pwm3 init success");
		}

    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL1, period, pulse);
		rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL2, period, pulse);
    /* 使能设备 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL1);
		rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL2);

    while (1)
    {
				rt_thread_mdelay(50);
				rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL1, period, 2000000);
				rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL2, period, 2000000);
    }
}

static int pwm_init(void)
{
    rt_thread_t tid = RT_NULL;
    /* 创建线程 1 */
    tid = rt_thread_create("pwm",
                            thread_entry, RT_NULL,
                            1024,
                            10, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    rt_pin_mode(DIR1_PIN, PIN_MODE_OUTPUT);
	  rt_pin_mode(DIR2_PIN, PIN_MODE_OUTPUT);
	
		rt_pin_write(DIR1_PIN, PIN_HIGH);
		rt_pin_write(DIR2_PIN, PIN_HIGH);
    return 0;
}
INIT_APP_EXPORT(pwm_init);
