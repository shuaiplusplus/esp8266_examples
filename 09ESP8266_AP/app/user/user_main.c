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


// �궨�塾�˴�����궨�塿
//==================================================================================
#define		ProjectName		"ESP8266_AP"		// �������궨��
#define 	AP_SSID  		"ESP8266"			//����wifi��
#define     AP_PASS         "403403403"			//����wifi����
//==================================================================================


// ȫ�ֱ������˴�����ȫ�ֱ�����
//==================================================================================
// ע��OS_Timer_1���붨��Ϊȫ�ֱ�������ΪESP8266���ں˻�Ҫʹ��
os_timer_t os_timer_1;	// �٣����������ʱ��(os_timer_t�ͽṹ��)
//==================================================================================


// ������ʱ����
//===========================================
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}
//===========================================

void ICACHE_FLASH_ATTR ESP8266_AP_Init_ws()
{
	struct softap_config AP_config_struct;    //AP�����ṹ��
	wifi_set_opmode(0x02);    //����ΪAPģʽ(Ĭ��Ҳ�Ǵ�ģʽ)�������浽Flash�У�operation mode
	//�ṹ�帳ֵ
	os_memset(&AP_config_struct, 0, sizeof(AP_config_struct));
	os_strcpy(AP_config_struct.ssid,AP_SSID);		// ����SSID(���ַ������Ƶ�ssid����)
	os_strcpy(AP_config_struct.password,AP_PASS);	// ��������(���ַ������Ƶ�password����)
	AP_config_struct.ssid_len=os_strlen(AP_SSID);	// ����ssid����(��SSID�ĳ���һ��)
	AP_config_struct.channel=1;                      		// ͨ����1��13
	AP_config_struct.authmode=AUTH_WPA2_PSK;           	// ���ü���ģʽ
	AP_config_struct.ssid_hidden=0;                  		// ������SSID
	AP_config_struct.max_connection=4;               		// ���������
	AP_config_struct.beacon_interval=100;            		// �ű���ʱ��100��60000 ms

	wifi_softap_set_config(&AP_config_struct);				// ����soft-AP�������浽Flash
}




// �����ʱ�Ļص�����
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// �ڣ�����ص�����
{
	struct ip_info ST_ESP8266_IP;	// IP��Ϣ�ṹ��

	u8  ESP8266_IP[4];		// ���ʮ������ʽ����IP


	// ��ѯ����ӡESP8266�Ĺ���ģʽ
	//---------------------------------------------------------------------
	switch(wifi_get_opmode())	// �������ģʽ
	{
		case 0x01:	os_printf("\nESP8266_Mode = Station\n");		break;
		case 0x02:	os_printf("\nESP8266_Mode = SoftAP\n");			break;
		case 0x03:	os_printf("\nESP8266_Mode = Station+SoftAP\n");	break;
	}


	// ��ȡESP8266_APģʽ�µ�IP��ַ
	//��APģʽ�£��������DHCP(Ĭ��)������δ����IP��ز�����ESP8266��IP��ַ=192.168.4.1��
	//-----------------------------------------------------------------------------------
	wifi_get_ip_info(SOFTAP_IF,&ST_ESP8266_IP);	// ����2��IP��Ϣ�ṹ��ָ��

	// ESP8266_AP_IP.ip.addr==32λ������IP��ַ������ת��Ϊ���ʮ���Ƶ���ʽ
	//------------------------------------------------------------------------------------------
	ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;			// ���ʮ����IP�ĵ�һ���� <==> addr�Ͱ�λ
	ESP8266_IP[1] = ST_ESP8266_IP.ip.addr>>8;		// ���ʮ����IP�ĵڶ����� <==> addr�εͰ�λ
	ESP8266_IP[2] = ST_ESP8266_IP.ip.addr>>16;		// ���ʮ����IP�ĵ������� <==> addr�θ߰�λ
	ESP8266_IP[3] = ST_ESP8266_IP.ip.addr>>24;		// ���ʮ����IP�ĵ��ĸ��� <==> addr�߰�λ

	// ��ӡESP8266��IP��ַ
	//-----------------------------------------------------------------------------------------------
	os_printf("ESP8266_IP = %d.%d.%d.%d\n",ESP8266_IP[0],ESP8266_IP[1],ESP8266_IP[2],ESP8266_IP[3]);


	// ��ѯ����ӡ�����WIFI���豸����
	//-----------------------------------------------------------------------------------------
	os_printf("Number of devices connected to this WIFI = %d\n",wifi_softap_get_station_num());

	os_printf("\r\n----OS_Timer_1_callback----\r\n");	// ����ص�������־
}
//======================================================================


//�����ʱ��os_timer��ʼ��������ms����
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_Init(u32 time_ms, bool repeat_flag)
{
    //�رն�ʱ��
	//����Ϊ��ʱ����ַ
	os_timer_disarm(&os_timer_1);

	//���ö�ʱ���ص�����,NULL��ʾ�޻ص������Ĳ���
	//����1��Ҫ���õĶ�ʱ��������2���ص�����������3���ص������Ĳ���
	os_timer_setfn(&os_timer_1, (os_timer_func_t *)os_timer_1_callback, NULL);

	//ʹ�ܶ�ʱ��(����)
	//����1 ��Ҫʹ�ܵĶ�ʱ��������2����ʱʱ�䣨ms��������3���Ƿ��ظ�  1-�ظ���2-���ظ�
	//���û�е���system_timer_reinit����֧�ֵķ�Χ�ǣ�5ms-6,870,947ms
	//���������system_timer_reinit����֧�ֵķ�Χ�ǣ�100ms-428,496ms
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
    uart_init(115200,115200);    //��ʼ�����ڲ�����
    os_delay_us(10000);          //�ȴ������ȶ�
	os_printf("\r\n=================================================\r\n");
	os_printf("\t Project:\t%s\r\n", ProjectName);
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	ESP8266_AP_Init_ws();    //��ʼ��ESP8266AP����
	os_timer_1_Init(1000, 1);    //os_timer1��ʼ����1�룬�ظ�

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

