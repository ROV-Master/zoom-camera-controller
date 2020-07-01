/**
 * @desc: UART2串口数据解析来自NnaoPi的变焦控制命令
 */

#include "uart.h"
#include "control.h"
#include <rtdevice.h>

#define LOG_TAG             "uart"
#include <drv_log.h>

#define CONTROL_UART_NAME   "uart2"

/* 控制数据 结构体 */
cmd_t cmd_data;

/* 上层控制器发送的变焦控制命令，如果命令数据包正常，则返回"ok"
 * 也可表示控制器 检测是否存在变焦组件，因此收到正常的数据包即返回"ok"
*/
rt_int8_t send_buff[2] = "ok";


/* 用于接收消息的信号量 */
static struct rt_semaphore rx_sem;
rt_device_t camera_uart_device;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

static void serial_thread_entry(void *parameter)
{
    unsigned char  ch; 
    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(camera_uart_device, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
				control_data_analysis(ch, &cmd_data);
    }
}

static int uart2_init(void)
{
    /* 查找系统中的串口设备 */
    camera_uart_device = rt_device_find(CONTROL_UART_NAME);
    if (!camera_uart_device)
    {
        LOG_E("find %s failed!\n", CONTROL_UART_NAME);
        return RT_ERROR;
    }
		else
				LOG_I("%s init success", camera_uart_device);


		/* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(camera_uart_device, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(camera_uart_device, uart_input);
		
		/* 开机发送该数据包，表示变焦组件正常 */
    rt_device_write(camera_uart_device, 0, send_buff, (sizeof(send_buff)));
    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("uart", serial_thread_entry, RT_NULL, 1024, 10, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

		return 0;
}
INIT_APP_EXPORT(uart2_init);

/**
 * @brief  控制命令解析
 * @param  data 数据字， *cmd控制命令结构体指针
 */
void control_data_analysis(rt_uint8_t data, cmd_t *cmd) //控制数据解析
{
	static rt_uint8_t i;
	static rt_uint8_t rxBuffer[10] = {0}; // 数据包
	static rt_uint8_t rxCheck = 0;        // 尾校验字
	static rt_uint8_t rxCount = 0;        // 接收计数
	
	rxBuffer[rxCount++] = data; // 将收到的数据存入缓冲区中
	if(rxCount > 9)
		rxCount = 0;

	if (rxBuffer[0] != 0xAA && rxBuffer[1] != 0x55) // 检测包头
	{
		// 数据头不对，则重新开始寻找0xAA、0X55数据头
		rxCount = 0; // 清空缓存区
		return;
	}
	if (rxCount < CONTROL_PACKET_LENGTH)
		return; // 数据不满6个，则返回

	/*********** 只有接收满11个字节数据 才会进入以下程序 ************/
	
	for (i = 0; i < CONTROL_PACKET_LENGTH - 1; i++)
		rxCheck += rxBuffer[i]; //校验位累加	
	
	if (rxCheck == rxBuffer[CONTROL_PACKET_LENGTH - 1]) // 判断数据包校验是否正确
	{

		cmd->zoom_dir  = (stepper_dir_e)rxBuffer[3];
		cmd->focus_dir = (stepper_dir_e)rxBuffer[4];
		
		cmd->focus_angle = STEP_ANGLE; // 每次转动STEP_ANGLE°
		cmd->zoom_angle  = STEP_ANGLE;
		/* 数据包正常，返回"ok" */
		rt_device_write(camera_uart_device, 0, send_buff, (sizeof(send_buff)));
	}	

	rxCount = 0; // 清空缓存区
	rxCheck = 0; // 校验位清零

}

