/**
 * @desc: 主控制线程
 */

#include "control.h"
#include "stepper.h"
#include "uart.h"

#define LOG_TAG             "control"
#include <drv_log.h>



/* 线程入口 */
static void control_thread_entry(void* parameter)
{
    
    while (1)
    {
			stepper_control(&focusStepper, &cmd_data.zoom_dir, &cmd_data.zoom_angle);
			stepper_control(&zoomStepper, &cmd_data.focus_dir, &cmd_data.focus_angle);
			
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
	{			
		rt_thread_startup(tid);
		
		/* 步进电机引脚初始化 */
		stepper_init(&zoomStepper);
		stepper_init(&focusStepper);
		LOG_I("stepper init");
	}
  return 0;
}
INIT_APP_EXPORT(control_init);
