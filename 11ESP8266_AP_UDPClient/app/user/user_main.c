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
#include "mem.h"
#include "ip_addr.h"
#include "espconn.h"


// 宏定义【此处定义宏定义】
//==================================================================================
#define		ProjectName		"ESP8266_AP_UDPClient"		// 工程名宏定义
#define 	AP_SSID  		"ESP8266"			//设置wifi名
#define     AP_PASS         "403403403"			//设置wifi密码
//==================================================================================


// 全局变量【此处定义全局变量】
//==================================================================================
// 注：OS_Timer_1必须定义为全局变量，因为ESP8266的内核还要使用
os_timer_t os_timer_1;	// ①：定义软件定时器(os_timer_t型结构体)
//==================================================================================


// 毫秒延时函数
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}



//初始化ESP8266为AP模式
void ICACHE_FLASH_ATTR ESP8266_AP_Init_ws()
{
	struct softap_config AP_config_struct;    //AP参数结构体
	wifi_set_opmode(0x02);    //设置为AP模式(默认也是此模式)，并保存到Flash中，operation mode
	//结构体赋值
	os_memset(&AP_config_struct, 0, sizeof(AP_config_struct));
	os_strcpy(AP_config_struct.ssid,AP_SSID);		// 设置SSID(将字符串复制到ssid数组)
	os_strcpy(AP_config_struct.password,AP_PASS);	// 设置密码(将字符串复制到password数组)
	AP_config_struct.ssid_len = os_strlen(AP_SSID);	// 设置ssid长度(和SSID的长度一致)
	AP_config_struct.channel = 1;                      		// 通道号1～13
	AP_config_struct.authmode = AUTH_WPA2_PSK;           	// 设置加密模式
	AP_config_struct.ssid_hidden = 0;                  		// 不隐藏SSID
	AP_config_struct.max_connection = 4;               		// 最大连接数
	AP_config_struct.beacon_interval = 100;            		// 信标间隔时槽100～60000 ms

	wifi_softap_set_config(&AP_config_struct);				// 设置soft-AP，并保存到Flash
}

//成功发送网络数据的回调函数
void ICACHE_FLASH_ATTR ESP8266_WIFI_Send_callback_ws(void *arg)
{
	os_printf("\nESP8266 WIFI SEND SUCCEED\n");
}


//成功接收网络数据的回调函数
void ICACHE_FLASH_ATTR ESP8266_WIFI_Recv_callback_ws(void *arg, char *pdata, unsigned short len)
{
    struct espconn * T_arg = arg;    //缓存网络连接结构体指针

    remot_info * P_port_info = NULL;    //远端连接信息结构体指针

    //根据数据设置LED的亮和灭
    if(pdata[0] == 'k' || pdata[0] == 'K')
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);    //将GPIO12设置为输出模式，并输出高电平
    else if(pdata[0] == 'g' || pdata[0] == 'G')
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //将GPIO12设置为输出模式，并输出低电平
    os_printf("\nESP8266 Receive Data = %s\n",pdata);

	// 获取远端信息【UDP通信是无连接的，向远端主机回应时需获取对方的IP/端口信息】
	//------------------------------------------------------------------------------------
	if(espconn_get_connection_info(T_arg, &P_port_info, 0) == ESPCONN_OK)	// 获取远端信息
	{
		T_arg->proto.udp->remote_port  = P_port_info->remote_port;		// 获取对方端口号
		T_arg->proto.udp->remote_ip[0] = P_port_info->remote_ip[0];		// 获取对方IP地址
		T_arg->proto.udp->remote_ip[1] = P_port_info->remote_ip[1];
		T_arg->proto.udp->remote_ip[2] = P_port_info->remote_ip[2];
		T_arg->proto.udp->remote_ip[3] = P_port_info->remote_ip[3];
		//os_memcpy(T_arg->proto.udp->remote_ip,P_port_info->remote_ip,4);	// 内存拷贝

		//串口打印远端的IP地址
		os_printf("remote ip: %d.%d.%d.%d",\
				T_arg->proto.udp->remote_ip[0],\
				T_arg->proto.udp->remote_ip[1], \
				T_arg->proto.udp->remote_ip[2], \
				T_arg->proto.udp->remote_ip[3]);

		espconn_send(T_arg, "ESP8266_WIFI_Recv_OK\n", os_strlen("ESP8266_WIFI_Recv_OK\n"));
	}
}


//定义espconn型结构体（网络连接结构体）
struct espconn ST_NetCon;

void ICACHE_FLASH_ATTR ESP8266_UDP_Init()
{
	ST_NetCon.type = ESPCONN_UDP;    //UDP协议

	ST_NetCon.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));    //申请内存

	ST_NetCon.proto.udp->local_port = 8266;    //设置ESP8266本地端口
	ST_NetCon.proto.udp->remote_port = 8888;    //设置UDP服务器的端口
	ST_NetCon.proto.udp->remote_ip[0] = 192;    //设置目标服务器的IP地址，第一个8位
	ST_NetCon.proto.udp->remote_ip[1] = 168;    //设置目标服务器的IP地址，第二个8位
	ST_NetCon.proto.udp->remote_ip[2] = 4;    //设置目标服务器的IP地址，第三个8位
	ST_NetCon.proto.udp->remote_ip[3] = 2;    //设置目标服务器的IP地址，第四个8位

	espconn_regist_sentcb(&ST_NetCon, ESP8266_WIFI_Send_callback_ws);    //注册网络数据发送成功的回调函数
	espconn_regist_recvcb(&ST_NetCon, ESP8266_WIFI_Recv_callback_ws);    //注册网络数据接收成功的回调函数

	espconn_create(&ST_NetCon);    //初始化UDP通信

	espconn_send(&ST_NetCon, "Hello From ESP8266\n", \
			os_strlen("Hello From ESP8266\n"));    //主动向UDP服务器发送数据
}


// 软件定时的回调函数
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// ②：定义回调函数
{
	struct ip_info ST_ESP8266_IP;	// IP信息结构体

	u8  ESP8266_IP[4];		// 点分十进制形式保存IP

	wifi_get_ip_info(SOFTAP_IF, &ST_ESP8266_IP);    //查询AP模式下ESP8266的IP地址

	if(ST_ESP8266_IP.ip.addr != 0)
	{
		ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;    //点分十进制的第一个数
		ESP8266_IP[1] = ST_ESP8266_IP.ip.addr >> 8;    //点分十进制的第二个数
		ESP8266_IP[2] = ST_ESP8266_IP.ip.addr >> 16;    //点分十进制的第三个数
		ESP8266_IP[3] = ST_ESP8266_IP.ip.addr >> 24;    //点分十进制的第四个数
	}
	//打印ESP8266的IP地址
	os_printf("ESP8266 IP = %d.%d.%d.%d", ESP8266_IP[0], ESP8266_IP[1], ESP8266_IP[2], ESP8266_IP[3]);

	os_timer_disarm(&os_timer_1);    //关闭定时器

	ESP8266_UDP_Init();    //初始化UDP网络连接

}




//软件定时器os_timer初始化函数【ms级】
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

//LED和KEY初始化
void ICACHE_FLASH_ATTR LED_Init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);    //设置GPIO12为通用IO,green
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);    //设置GPIO13为通用IO,blue
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);    //设置GPIO15为通用IO,red
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO12);    //设置GPIO4为通用IO,key

    //设置为输出模式
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //将GPIO12设置为输出模式，并输出低电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);    //将GPIO13设置为输出模式，并输出低电平
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //将GPIO15设置为输出模式，并输出高电平
    //设置为输入模式
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(4));			//将GPIO4设置为输入模式，默认为输入状态

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

	LED_Init();    //初始化LED和KEY

	ESP8266_AP_Init_ws();    //初始化ESP8266AP设置
	os_timer_1_Init(30000, 0);    //os_timer1初始化，30秒，单次，为了给主机服务器小软件准备时间

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

