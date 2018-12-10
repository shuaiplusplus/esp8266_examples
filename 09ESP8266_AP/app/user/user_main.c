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
#define		ProjectName		"ESP8266_AP"		// 工程名宏定义
#define 	AP_SSID  		"ESP8266"			//设置wifi名
#define     AP_PASS         "403403403"			//设置wifi密码
//==================================================================================


// 全局变量【此处定义全局变量】
//==================================================================================
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

void ICACHE_FLASH_ATTR ESP8266_AP_Init_ws()
{
	struct softap_config AP_config_struct;    //AP参数结构体
	wifi_set_opmode(0x02);    //设置为AP模式(默认也是此模式)，并保存到Flash中，operation mode
	//结构体赋值
	os_memset(&AP_config_struct, 0, sizeof(AP_config_struct));
	os_strcpy(AP_config_struct.ssid,AP_SSID);		// 设置SSID(将字符串复制到ssid数组)
	os_strcpy(AP_config_struct.password,AP_PASS);	// 设置密码(将字符串复制到password数组)
	AP_config_struct.ssid_len=os_strlen(AP_SSID);	// 设置ssid长度(和SSID的长度一致)
	AP_config_struct.channel=1;                      		// 通道号1～13
	AP_config_struct.authmode=AUTH_WPA2_PSK;           	// 设置加密模式
	AP_config_struct.ssid_hidden=0;                  		// 不隐藏SSID
	AP_config_struct.max_connection=4;               		// 最大连接数
	AP_config_struct.beacon_interval=100;            		// 信标间隔时槽100～60000 ms

	wifi_softap_set_config(&AP_config_struct);				// 设置soft-AP，并保存到Flash
}




// 软件定时的回调函数
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// ②：定义回调函数
{
	struct ip_info ST_ESP8266_IP;	// IP信息结构体

	u8  ESP8266_IP[4];		// 点分十进制形式保存IP


	// 查询并打印ESP8266的工作模式
	//---------------------------------------------------------------------
	switch(wifi_get_opmode())	// 输出工作模式
	{
		case 0x01:	os_printf("\nESP8266_Mode = Station\n");		break;
		case 0x02:	os_printf("\nESP8266_Mode = SoftAP\n");			break;
		case 0x03:	os_printf("\nESP8266_Mode = Station+SoftAP\n");	break;
	}


	// 获取ESP8266_AP模式下的IP地址
	//【AP模式下，如果开启DHCP(默认)，并且未设置IP相关参数，ESP8266的IP地址=192.168.4.1】
	//-----------------------------------------------------------------------------------
	wifi_get_ip_info(SOFTAP_IF,&ST_ESP8266_IP);	// 参数2：IP信息结构体指针

	// ESP8266_AP_IP.ip.addr==32位二进制IP地址，将它转换为点分十进制的形式
	//------------------------------------------------------------------------------------------
	ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;			// 点分十进制IP的第一个数 <==> addr低八位
	ESP8266_IP[1] = ST_ESP8266_IP.ip.addr>>8;		// 点分十进制IP的第二个数 <==> addr次低八位
	ESP8266_IP[2] = ST_ESP8266_IP.ip.addr>>16;		// 点分十进制IP的第三个数 <==> addr次高八位
	ESP8266_IP[3] = ST_ESP8266_IP.ip.addr>>24;		// 点分十进制IP的第四个数 <==> addr高八位

	// 打印ESP8266的IP地址
	//-----------------------------------------------------------------------------------------------
	os_printf("ESP8266_IP = %d.%d.%d.%d\n",ESP8266_IP[0],ESP8266_IP[1],ESP8266_IP[2],ESP8266_IP[3]);


	// 查询并打印接入此WIFI的设备数量
	//-----------------------------------------------------------------------------------------
	os_printf("Number of devices connected to this WIFI = %d\n",wifi_softap_get_station_num());

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

	ESP8266_AP_Init_ws();    //初始化ESP8266AP设置
	os_timer_1_Init(1000, 1);    //os_timer1初始化，1秒，重复

	os_printf("\r\n==================user init end=================\r\n");
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

