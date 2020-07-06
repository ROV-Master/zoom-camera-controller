/**
 * @desc: 系统状态指示灯
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LOG_TAG             "main"
#include <drv_log.h>

/* defined the LED0 pin: PA1 */
#define LED0_PIN    GET_PIN(A, 1)

// main函数中仅闪烁系统指示灯
int main(void)
{
  /* set LED0 pin mode to output */
  rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

	while (1) 
	{
		rt_pin_write(LED0_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED0_PIN, PIN_LOW);
		rt_thread_mdelay(500);
	}
}
