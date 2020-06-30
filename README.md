<div align="center">
  <a href="https://github.com/zengwangfa/rov-master"><img src="https://zengwangfa.oss-cn-shanghai.aliyuncs.com/rov/rovmaster(vector)1.png" alt=""></a>
  <a href="https://github.com/zengwangfa/rov-master"><h1>ROV-Master</h2></a>
</div>

<div align="center">
  <a href="https://www.stmcu.com.cn"><img src="https://img.shields.io/badge/Device-STM32F103C8T6-orange?style=flat-square" alt="STM32"></a>
  <a href="https://www.rt-thread.org/"><img src="https://img.shields.io/badge/OS-RT--Thread-brightgreen" ></a>
  <a href="https://img.shields.io"><img src="https://img.shields.io/github/repo-size/ROV-Master/zoom-camera-controller?style=flat-square" ></a>
</div>

## 1.简介

>  主机通过 `UART` 下发命令，控制镜头上的两个4相步进电机实现聚焦（focus）与变焦（zoom）。



## 2.使用说明
![变焦镜头步进电机励磁序列](https://zengwangfa.oss-cn-shanghai.aliyuncs.com/rov/focus_camera_sequence_of_excitation.png "变焦镜头步进电机励磁序列")



该步进电机励磁序列节拍表如上图，由此可以定义出其 **4拍正反转表对应的数组：**

```c
static rt_uint8_t F_Rotation[4] = {0x09,0x0A,0x06,0x05} ;  // 4节拍正转表 Forward
static rt_uint8_t R_Rotation[4] = {0x05,0x06,0x0A,0x09} ;  // 4节拍反转表 Reverse
```

因此其对应的控制程序为：

```c

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

```



### 2.1 接线说明



### 2.2 协议说明

- [x] 以下为ROV发送给变焦摄像头设备的数据包定义(16进制)：

|     数组编号     |  0   |  1   |     2      |    3     |    4     |     5      |
| :--------------: | :--: | :--: | :--------: | :------: | :------: | :--------: |
| 数据所代表的含义 | 包头 | 包头 | 数据长度位 | 聚焦指令 | 变焦指令 | 累加校验和 |
|     具体描述     |  AA  |  55  |     02     |    xx    |    xx    |    SUM     |

- 聚焦指令为`0x01`时，为聚焦（focus）放大
- 聚焦指令为`0x02`时，为聚焦（focus）缩小

---

- 变焦指令为`0x01`时，为变焦（zoom）拉近

- 变焦指令为`0x02`时，为变焦（zoom）拉远


## 3. 进度
- 驱动层
	- [x] Stepper​ :motorcycle:
	- [x] UART
	- [ ] Watch-Dog :dog2:

- 应用层
	- [ ] 驱动控制程序 :wrench:
	- [x] UART 控制协议 :pencil:
	- [x] Msh 调试 :dizzy:
	
## 4.参考

- [【快速搭建】Env 创建 RT-Thread 项目工程](https://www.rt-thread.org/document/site/application-note/setup/standard-project/an0017-standard-project/)

- [RT-Thread 添加设备框架流程(PWM示例)](https://www.rt-thread.org/document/site/application-note/driver/pwm/an0037-rtthread-driver-pwm/#)

  

  