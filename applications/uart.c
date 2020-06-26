/**
 * @desc: UART2串口数据解析来自NnaoPi的变焦控制命令
 */

#include "uart.h"
#include <rtdevice.h>

#define LOG_TAG             "uart"
#include <drv_log.h>

#define CONTROL_UART_NAME   "uart2"

/* 用于接收消息的信号量 */
static struct rt_semaphore rx_sem;
static rt_device_t serial;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

static void serial_thread_entry(void *parameter)
{
    char ch;

    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        /* 读取到的数据通过串口错位输出 */
        ch = ch + 1;
        rt_device_write(serial, 0, &ch, 1);
    }
}

static int uart2_init(void)
{
		rt_thread_t tid;
	
    /* 查找系统中的串口设备 */
    serial = rt_device_find(CONTROL_UART_NAME);
    if (!serial)
    {
        LOG_E("find %s failed!\n", CONTROL_UART_NAME);
        return RT_ERROR;
    }
		else
		{
				LOG_I("%s init success", serial);
		}

    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);

    /* 创建 serial 线程 */
		tid = rt_thread_create("uart",
												serial_thread_entry, RT_NULL,
												1024,
												8, 10);

    if (tid != RT_NULL)      
				rt_thread_startup(tid);

		return 0;
}
INIT_APP_EXPORT(uart2_init);
