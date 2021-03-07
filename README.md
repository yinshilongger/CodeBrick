# CodeBrick

## 介绍
一种无OS的MCU实用软件框架，包括任务轮询管理，命令管理器、低功耗管理、环形缓冲区等实用模块。系统中广泛利用自定义段技术减少各个模块间的耦合关系，大大提供程序的可维护性。

## 主要功能
 - 支持模块自动化管理，并提供不同优先等级初始化声明接口。
 - 支持任务轮询管理，通过简单的宏声明即可实现，不需要复杂的声明调用。
 - 支持低功耗管理，休眠与唤醒通知。
 - 支持命令行解析，命令注册与执行。
 - blink设备支持，统一管理LED、震动马达、蜂鸣器
## 使用说明

完整的代码可以参考工程文件,系统开发平台如下：

MCU：STM32F401RET6

IDE：IAR 7.4或者Keil MDK 4.72A

### 任务初始化及任务轮询管理(module)

使用此模块前需要系统提供滴答定时器，用于驱动任务轮询作业。（参考platform.c）

```c
//定时器中断(提供系统滴答)
void SysTick_Handler(void)
{
    systick_increase(SYS_TICK_INTERVAL); //增加系统节拍
}
```

注册初始化入口及任务(参考自key_task.c)

```c
static void key_init(void)
{
    /*do something*/
}

static void key_scan(void)
{
    /*do something*/
}

module_init("key", key_init);              //注册按键初始化接口
driver_register("key", key_scan, 20);      //注册按键任务(20ms轮询1次)
```

### 命令管理器(cli)
适用于在线调试、参数配置等(参考使用cli_task.c)，用户可以通过串口输出命令行控制设备行为、查询设备状态等功能。

#### 命令格式

cli支持的命令行格式如下：

&lt;cmd name&gt; &lt; param1&gt; &lt; param2&gt; &lt; paramn&gt;  &lt; \r\n &gt;
&lt;cmd name&gt; ,&lt; param1&gt;, &lt; param2&gt;, &lt; paramn&gt;,  &lt; \r\n &gt;


每行命令包含一个命令名称+命令参数(可选),命令名称及参数可以通过空格或者','进行分隔。

#### 系统默认命令

cli系统自带了2条默认命令，分别是"?"与"help"命令，输入他们可以列出当前系统包含的命令列表，如下所示：

```C
?         - alias for 'help'
help      - list all command.
pm        - Low power control command
reset     - reset system
sysinfo   - show system infomation.

```

#### 适配命令管理器
完整的例子可以参考cli_task.c.

```c
static cli_obj_t cli;                               /*命令管理器对象 */

/* 
 * @brief       命令行任务初始化
 * @return      none
 */ 
static void cli_task_init(void)
{
    cli_port_t p = {tty.write, tty.read};           /*读写接口 */
    
    cli_init(&cli, &p);                             /*初始化命令行对象 */
     
    cli_enable(&cli);
    
    cli_exec_cmd(&cli,"sysinfo");                   /*显示系统信息*/
}

/* 
 * @brief       命令行任务处理
 * @return      none
 */ 
static void cli_task_process(void)
{
    cli_process(&cli);
}

module_init("cli", cli_task_init);                  
task_register("cli", cli_task_process, 10);          /*注册命令行任务*/
```

#### 命令注册

以复位命令为例(参考cmd_devinfo.c)：

```C
#include "cli.h"
//...
/* 
 * @brief       复位命令
 */ 
int do_cmd_reset(struct cli_obj *o, int argc, char *argv[])
{
    NVIC_SystemReset();
    return 0;
}cmd_register("reset",do_cmd_reset, "reset system");

```

### 低功耗管理器(pm)

控制间歇运行，降低系统功耗。其基本的工作原理是通过轮询系统中各个模块是否可以允许系统进入低功耗。实际上这是一种判决机制，所有模块都具有有票否决权，即只要有一个模块不允许休眠，那么系统就不会进入休眠状态。pm模块在休眠前会统计出各个模块会返回最小允许休眠时长，并以最小休眠时长为单位进行休眠。

#### 如何适配

使用前需要通过pm_init进行初始化适配，并提供当前系统允许的最大休眠时间，进入休眠的函数接口，基本的接口定义如下：
```C
/*低功耗适配器 ---------------------------------------------------------*/
typedef struct {
    /**
     * @brief    系统最大休眠时长(ms)
     */  
    unsigned int max_sleep_time;
    /**
     * @brief     进入休眠状态
     * @param[in] time - 期待休眠时长(ms)
     * @retval    实际休眠时长
     * @note      休眠之后需要考虑两件事情,1个是需要定时起来给喂看门狗,否则会在休眠
     *            期间发送重启.另外一件事情是需要补偿休眠时间给系统滴答时钟,否则会
     *            造成时间不准。
     */     
    unsigned int (*goto_sleep)(unsigned int time);
}pm_adapter_t;
void pm_init(const pm_adapter_t *adt);

void pm_enable(void);

void pm_disable(void);

void pm_process(void);
```
完成的使用例子可以参考platform-lowpower.c，默认情况下是禁用低功耗功能的，读者可以去除工程中原来不带低功耗版本的platform.c，并加入platform-lowpower.c文件进行编译即可使用。

#### 注册低功耗设备

以按键扫描为例，正常情况下，如果按键没有按下，那么系统休眠可以进入休眠状态，对按键功能是没有影响的。如果按键按下时，那么系统需要定时唤醒并轮询按键任务。

所以在一个低功耗系统下，为了不影响按键实时性需要处理好两个事情：

1. 系统休眠状态下，如果有按键按下，那系统系统应立即唤醒，以便处理接下来的扫描工作。
2. 如果按键按下时，系统可以进入休眠，但需要定时唤醒起来轮询按键任务。

对于第一种情况，将按键配置为边沿中断唤醒即可，以STM32F4为例(参考key_task.c)，它支持外部中断唤醒功能。
```C
/* 
 * @brief       按键 io初始化
 *              PC0 -> key;
 * @return      none
 */ 
static void key_io_init(void)
{
    /* Enable GPIOA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    gpio_conf(GPIOC, GPIO_Mode_IN, GPIO_PuPd_UP, GPIO_Pin_0);
    
    //低功耗模式下,为了能够检测到按键，配置为中断唤醒
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);
    exti_conf(EXTI_Line0, EXTI_Trigger_Falling, ENABLE);
    nvic_conf(EXTI0_IRQn, 0x0F, 0x0F);
    
    key_create(&key, readkey, key_event);            /*创建按键*/
}
```

对于第二种情况，可以通过pm_dev_register来处理，当系统请求休眠时，如果此时按键按下，则返回下次唤醒时间即可，如下面的例子所示。
```C
//参考key_task.c
#include "pm.h"                                     
/*
 * @brief	   休眠通知
 */
static unsigned int  key_sleep_notify(void)
{
    return key_busy(&key) || readkey() ? 20 : 0;    /* 非空闲时20ms要唤醒1次*/
} pm_dev_register("key", NULL, key_sleep_notify, NULL);

```

### blink模块
具有闪烁特性(led, motor, buzzer)的设备(led, motor, buzzer)管理

使用步骤:

- 需要系统提供滴答时钟，blick.c中是通过get_tick()接口获取，依赖module模块
- 需要在任务中定时进行轮询

或者通过"module"模块的任务注册来实现

```c
task_register("blink", blink_dev_process, 50);  //50ms轮询1次
```
#### LED驱动
```c
blink_dev_t led;                             //定义led设备

/*
 *@brief     红色LED控制(GPIOA.8)
 *@param[in] on - 亮灭控制
 */
static void led_ctrl(int on)
{
    if (on)
        GPIOA->ODR |= (1 << 8);
    else 
        GPIOA->ODR &= ~(1 << 8);
}

/*
 *@brief     led初始化程序
 */
void led_init(void)
{
    led_io_init(void);                  //led io初始化
    blink_dev_create(&led, led_ctrl);   //创建led设备
    
    blink_dev_ctrl(&led, 50, 100, 0);   //快闪(50ms亮, 100ms灭)
}
```


### 按键管理模块
类似blink模块，使用之前有两个注意事项:
- 需要系统提供滴答时钟，key.c中是通过get_tick()接口获取，依赖module模块
- 需要在任务中定时进行轮询

```c
key_t key;                             //定义按键管理器

/*
 *@brief     按键事件
 *@param[in] type     - 按键类型(KEY_PRESS, KEY_LONG_DOWN, KEY_LONG_UP)  
 *@param[in] duration - 长按持续时间
 */
void key_event(int type, unsigned int duration)
{
	if (type == KEY_PRESS) {                //短按
		 
	} else if (type == KEY_LONG_DOWN) {     //长按
		
	}
} 

//读取键值(假设按键输出口为STM32 MCU PA8)
int read_key(void)
{
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_RESET;
}

/*
 *@brief     按键初始化
 */
void key_init(void)
{
    //打开GPIO 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//配置成输入模式
    gpio_conf(GPIOA, GPIO_Mode_IN, GPIO_PuPd_NOPULL, GPIO_Pin_8); 
    //创建1个按键
    key_create(&key, read_key, key_event);  
}

```
