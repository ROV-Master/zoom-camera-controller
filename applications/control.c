/**
 * @desc: 主控制线程(及看门狗初始化)
 */

#include "control.h"
#include "stepper.h"
#include "uart.h"

#define LOG_TAG             "control"
#include <drv_log.h>

#define WDT_DEVICE_NAME    "wdt"    /* 看门狗设备名称 */

static rt_device_t wdg_dev;    /* 看门狗设备句柄 */

/* 线程入口 */
static void control_thread_entry(void* parameter)
{
  
	while (1)
	{
		stepper_control(&focusStepper);
		stepper_control(&zoomStepper);
		
		rt_thread_mdelay(10); // 此处挂起线程，以防卡死其他线程
	}
}

static int control_init(void)
{
	rt_thread_t tid = RT_NULL;
	/* 创建线程 */
	tid = rt_thread_create("control",
													control_thread_entry, 
													RT_NULL,
													1024,
													10, 10);
  if (tid != RT_NULL)		
		rt_thread_startup(tid);
		
	/* 步进电机引脚初始化 */
	stepper_init(&zoomStepper);
	stepper_init(&focusStepper);
	LOG_I("stepper init");
  return 0;
}
INIT_APP_EXPORT(control_init);


/************************************************ 看门狗线程 **********************************************/

static void wdg_thread_entry(void *parameter)
{
	while(1)
	{
		rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
		rt_thread_mdelay(100); // 100ms 喂狗一次
	}
}

static int wdg_init(void)
{
	rt_thread_t tid = RT_NULL;
	rt_err_t ret = RT_EOK;
	rt_uint32_t timeout = 1;  /* 溢出时间，单位：秒 */
	
	/* 根据设备名称查找看门狗设备，获取设备句柄 */
	wdg_dev = rt_device_find(WDT_DEVICE_NAME);
	if (!wdg_dev)
	{
		LOG_E("find %s failed!\n", WDT_DEVICE_NAME);
		return RT_ERROR;
	}
	/* 初始化设备 */
	ret = rt_device_init(wdg_dev);
	if (ret != RT_EOK)
	{
		rt_kprintf("initialize %s failed!\n", WDT_DEVICE_NAME);
		return RT_ERROR;
	}
	/* 设置看门狗溢出时间 */
	ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
	if (ret != RT_EOK)
	{
		rt_kprintf("set %s timeout failed!\n", WDT_DEVICE_NAME);
		return RT_ERROR;
	}
	/* 启动看门狗 */
	ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
	if (ret != RT_EOK)
	{
		rt_kprintf("start %s failed!\n", WDT_DEVICE_NAME);
		return -RT_ERROR;
	}
	/* 创建线程 进行喂狗*/
	tid = rt_thread_create("control",
													wdg_thread_entry, 
													RT_NULL,
													1024,
													30, 10);
  if (tid != RT_NULL)
	{			
		rt_thread_startup(tid);
		LOG_I("%s init", wdg_dev);
	}
	
  return ret;
}
INIT_APP_EXPORT(wdg_init);

