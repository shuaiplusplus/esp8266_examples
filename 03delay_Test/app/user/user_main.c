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
#define		ProjectName		"delay_Test"		// 工程名宏定义
//==================================================================================


// 全局变量【此处定义全局变量】
//==================================================================================
bool F_LED = 0;				// LED状态标志位
// 注：OS_Timer_1必须定义为全局变量，因为ESP8266的内核还要使用
os_timer_t os_timer_1;	// ①：定义软件定时器(os_timer_t型结构体)
//==================================================================================


// 毫秒延时函数
//===========================================
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}
//===========================================



//LED初始化函数
//RED GPIO15;GREEN GPIO12;BLUE GPIO13.
void ICACHE_FLASH_ATTR LED_Init(void)
{
    //管脚功能选择，设置为通用IO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U ,FUNC_GPIO12);    //设置GPIO12为通用IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U ,FUNC_GPIO13);    //设置GPIO13为通用IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U ,FUNC_GPIO15);    //设置GPIO15为通用IO
    //设置为输出模式，并输出高电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);    //将GPIO12设置为输出模式，并输出高电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);    //将GPIO12设置为输出模式，并输出高电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //将GPIO12设置为输出模式，并输出高电平
}


// 软件定时的回调函数
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// ②：定义回调函数
{
	F_LED = !F_LED;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED状态翻转
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),F_LED);		// blue  LED状态翻转
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),!F_LED);		// red   LED状态翻转


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

	LED_Init();
	//os_timer1初始化，500毫秒
	//os_timer_1_Init(500, 1);    //delay_Test用不到，注释掉

	os_printf("\r\n==================user init end=================\r\n");

	while(1)    //只这样会造成看门狗复位（只有while(1)）
	{
		system_soft_wdt_feed(); //这样就不会造成看门狗复位，但是会导致程序不再响应软定时器os_timer的回调（如果while(1)里面只有这一句）
		F_LED = !F_LED;
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED状态翻转
		os_printf("hello world\r\n");    //输出hello world
		delay_ms(1000);
	}
}


