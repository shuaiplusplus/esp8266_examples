/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"
#include <driver/uart.h>


// 宏定义【此处定义宏定义】
//==================================================================================
#define		ProjectName		"HW_timer"		// 工程名宏定义
//==================================================================================


// 全局变量【此处定义全局变量】
//==================================================================================
bool F_LED = 0;				// LED状态标志位
// 注：OS_timer_1必须定义为全局变量，因为ESP8266的内核还要使用
os_timer_t os_timer_1;	// ①：定义软件定时器(os_timer_t型结构体)
//==================================================================================


// 毫秒延时函数
//===========================================
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}
//===========================================



//IO初始化函数
//RED GPIO15;GREEN GPIO12;BLUE GPIO13;KEY GPIO4
void ICACHE_FLASH_ATTR LED_Init(void)
{
    //管脚功能选择，设置为通用IO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U ,FUNC_GPIO12);    //设置GPIO12为通用IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U ,FUNC_GPIO13);    //设置GPIO13为通用IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U ,FUNC_GPIO15);    //设置GPIO15为通用IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U ,FUNC_GPIO4);    //设置GPIO4为通用IO
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO4_U);    				//屏蔽GPIO4的上拉，因为外部已经上拉至高电平
    //设置为输出模式
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //将GPIO12设置为输出模式，并输出低电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);    //将GPIO13设置为输出模式，并输出低电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //将GPIO15设置为输出模式，并输出高电平
    //设置为输入模式
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(4));			//将GPIO4设置为输入模式，默认为输入状态
}


//外部中断服务程序（注意不要加ICACHE_FLASH_ATTR）
//======================================================================
void gpio_intr_handler(void)
{
    u32 gpio_status = 0;
    os_printf("enter exti intr\r\n");						  //进入中断打印一行数据来判断
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);         //把中断发生的IO找出来
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);    //清中断
    if(gpio_status & (BIT(4)))    //判断是不是GPIO4对应的下降沿中断,bit(4)= 10000b，1<<4
    {
		os_printf("key pressed\r\n");    //打印一行数据判断是否是按键按下了
    	F_LED = !F_LED;    //取反状态
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED状态翻转
    }
}
//======================================================================


//外部中断初始化函数
void ICACHE_FLASH_ATTR EXTI_init(void)
{
	ETS_GPIO_INTR_DISABLE();    //禁止所有IO中断
	ETS_GPIO_INTR_ATTACH((ets_isr_t)gpio_intr_handler,NULL);   //注册中断服务函数
	gpio_pin_intr_state_set(GPIO_ID_PIN(4),GPIO_PIN_INTR_NEGEDGE);    //配置为下降沿触发方式
	ETS_GPIO_INTR_ENABLE();    //使能GPIO中断
}



// 软件定时的回调函数
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// ②：定义回调函数
{
	F_LED = !F_LED;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED状态翻转
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),F_LED);		// blue  LED状态翻转
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED状态翻转


	os_printf("\r\n----OS_Timer_1_callback----\r\n");	// 进入回调函数标志
}
//======================================================================


//软件定时器os_timer初始化函数【ms级】
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_Init(u32 time_ms, bool repeat_flag)
{
    //关闭定时器
	//参数为定时器地址
	os_timer_disarm(&os_timer_1);

	//设置定时器回调函数,NULL表示无回调函数的参数
	//参数1：要设置的定时器；参数2：回调函数；参数3：回调函数的参数
	os_timer_setfn(&os_timer_1, (os_timer_func_t *)os_timer_1_callback, NULL);

	//使能定时器(毫秒)
	//参数1 ：要使能的定时器；参数2：定时时间（ms）；参数3：是否重复  1-重复，2-不重复
	//如果没有调用system_timer_reinit，可支持的范围是：5ms-6,870,947ms
	//如果调用了system_timer_reinit，可支持的范围是：100ms-428,496ms
	os_timer_arm(&os_timer_1, time_ms, repeat_flag);
}

//硬件定时器回调函数
void hw_timer_intr_callback(void)
{
	os_printf("hw_timer in\r\n");    //打印一行数据判断是否是按键按下了
	F_LED = !F_LED;    //取反状态
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED状态翻转
}
/* 硬件定时器初始化设置函数（整理在一起）区别于它调用的hw_timer_Init()函数
 * 使用注意：
 * 1.如果是NMI中断源，且为自动填装的定时器，调用hw_timer_arm时参数val必须大于100
 * 2.如果使用NMI中断源，那么定时器将为最高优先级，可打断其他ISR
 * 3.如果使用FRC1中断源，那么该定时器无法打断其他ISR
 * 4.hw_timer.c的接口不能跟PWM驱动接口函数同时使用，因为两者共用了同一个硬件定时器
 * 5.使用hw_timer.c接口，请勿调用wifi_set_sleep_type(LIGHT_SLEEP),否则不能响应NMI中断
 * 6.使用FRC1中断源，取值范围50-1677721us；使用NMI中断源，取值范围：100-1677721us
 */
void ICACHE_FLASH_ATTR hw_timer_Init(void)
{
	//初始化硬件ISR定时器
	//第一个参数：FRC1_SOURCE：使用FRC1中断源；NMI_SOURCE：使用NMI中断源；
	//第二个参数：0 不自动填装；1 自动填装
	hw_timer_init(0 ,1);    //使用FRC1中断源，自动填装
	hw_timer_set_func(hw_timer_intr_callback);    //设置硬件定时器中断回调函数
	hw_timer_arm(500000);    //设置定时时间为500ms
}



//======================================================================
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
            rf_cal_sec = 512 - 5;
            break;
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}



void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(115200,115200);    //初始化串口波特率
    os_delay_us(10000);          //等待串口稳定
	os_printf("\r\n=================================================\r\n");
	os_printf("\t Project:\t%s\r\n", ProjectName);
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	LED_Init();    //LED初始化
	//os_timer1初始化，500毫秒，重复
	//os_timer_1_Init(500, 1);   //这个例程用不到os_timer，屏蔽之
	//EXTI_init();    //外部中断初始化，此例程用不到，屏蔽之
	hw_timer_Init();    //硬件定时器初始化
	os_printf("\r\n==================user init end=================\r\n");

	/*while(1)
	{
		system_soft_wdt_feed();    //喂狗
	}*/
}

