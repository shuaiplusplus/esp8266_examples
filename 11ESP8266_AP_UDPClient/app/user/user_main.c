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


// �궨�塾�˴�����궨�塿
//==================================================================================
#define		ProjectName		"ESP8266_AP_UDPClient"		// �������궨��
#define 	AP_SSID  		"ESP8266"			//����wifi��
#define     AP_PASS         "403403403"			//����wifi����
//==================================================================================


// ȫ�ֱ������˴�����ȫ�ֱ�����
//==================================================================================
// ע��OS_Timer_1���붨��Ϊȫ�ֱ�������ΪESP8266���ں˻�Ҫʹ��
os_timer_t os_timer_1;	// �٣����������ʱ��(os_timer_t�ͽṹ��)
//==================================================================================


// ������ʱ����
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}



//��ʼ��ESP8266ΪAPģʽ
void ICACHE_FLASH_ATTR ESP8266_AP_Init_ws()
{
	struct softap_config AP_config_struct;    //AP�����ṹ��
	wifi_set_opmode(0x02);    //����ΪAPģʽ(Ĭ��Ҳ�Ǵ�ģʽ)�������浽Flash�У�operation mode
	//�ṹ�帳ֵ
	os_memset(&AP_config_struct, 0, sizeof(AP_config_struct));
	os_strcpy(AP_config_struct.ssid,AP_SSID);		// ����SSID(���ַ������Ƶ�ssid����)
	os_strcpy(AP_config_struct.password,AP_PASS);	// ��������(���ַ������Ƶ�password����)
	AP_config_struct.ssid_len = os_strlen(AP_SSID);	// ����ssid����(��SSID�ĳ���һ��)
	AP_config_struct.channel = 1;                      		// ͨ����1��13
	AP_config_struct.authmode = AUTH_WPA2_PSK;           	// ���ü���ģʽ
	AP_config_struct.ssid_hidden = 0;                  		// ������SSID
	AP_config_struct.max_connection = 4;               		// ���������
	AP_config_struct.beacon_interval = 100;            		// �ű���ʱ��100��60000 ms

	wifi_softap_set_config(&AP_config_struct);				// ����soft-AP�������浽Flash
}

//�ɹ������������ݵĻص�����
void ICACHE_FLASH_ATTR ESP8266_WIFI_Send_callback_ws(void *arg)
{
	os_printf("\nESP8266 WIFI SEND SUCCEED\n");
}


//�ɹ������������ݵĻص�����
void ICACHE_FLASH_ATTR ESP8266_WIFI_Recv_callback_ws(void *arg, char *pdata, unsigned short len)
{
    struct espconn * T_arg = arg;    //�����������ӽṹ��ָ��

    remot_info * P_port_info = NULL;    //Զ��������Ϣ�ṹ��ָ��

    //������������LED��������
    if(pdata[0] == 'k' || pdata[0] == 'K')
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);    //��GPIO12����Ϊ���ģʽ��������ߵ�ƽ
    else if(pdata[0] == 'g' || pdata[0] == 'G')
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //��GPIO12����Ϊ���ģʽ��������͵�ƽ
    os_printf("\nESP8266 Receive Data = %s\n",pdata);

	// ��ȡԶ����Ϣ��UDPͨ���������ӵģ���Զ��������Ӧʱ���ȡ�Է���IP/�˿���Ϣ��
	//------------------------------------------------------------------------------------
	if(espconn_get_connection_info(T_arg, &P_port_info, 0) == ESPCONN_OK)	// ��ȡԶ����Ϣ
	{
		T_arg->proto.udp->remote_port  = P_port_info->remote_port;		// ��ȡ�Է��˿ں�
		T_arg->proto.udp->remote_ip[0] = P_port_info->remote_ip[0];		// ��ȡ�Է�IP��ַ
		T_arg->proto.udp->remote_ip[1] = P_port_info->remote_ip[1];
		T_arg->proto.udp->remote_ip[2] = P_port_info->remote_ip[2];
		T_arg->proto.udp->remote_ip[3] = P_port_info->remote_ip[3];
		//os_memcpy(T_arg->proto.udp->remote_ip,P_port_info->remote_ip,4);	// �ڴ濽��

		//���ڴ�ӡԶ�˵�IP��ַ
		os_printf("remote ip: %d.%d.%d.%d",\
				T_arg->proto.udp->remote_ip[0],\
				T_arg->proto.udp->remote_ip[1], \
				T_arg->proto.udp->remote_ip[2], \
				T_arg->proto.udp->remote_ip[3]);

		espconn_send(T_arg, "ESP8266_WIFI_Recv_OK\n", os_strlen("ESP8266_WIFI_Recv_OK\n"));
	}
}


//����espconn�ͽṹ�壨�������ӽṹ�壩
struct espconn ST_NetCon;

void ICACHE_FLASH_ATTR ESP8266_UDP_Init()
{
	ST_NetCon.type = ESPCONN_UDP;    //UDPЭ��

	ST_NetCon.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));    //�����ڴ�

	ST_NetCon.proto.udp->local_port = 8266;    //����ESP8266���ض˿�
	ST_NetCon.proto.udp->remote_port = 8888;    //����UDP�������Ķ˿�
	ST_NetCon.proto.udp->remote_ip[0] = 192;    //����Ŀ���������IP��ַ����һ��8λ
	ST_NetCon.proto.udp->remote_ip[1] = 168;    //����Ŀ���������IP��ַ���ڶ���8λ
	ST_NetCon.proto.udp->remote_ip[2] = 4;    //����Ŀ���������IP��ַ��������8λ
	ST_NetCon.proto.udp->remote_ip[3] = 2;    //����Ŀ���������IP��ַ�����ĸ�8λ

	espconn_regist_sentcb(&ST_NetCon, ESP8266_WIFI_Send_callback_ws);    //ע���������ݷ��ͳɹ��Ļص�����
	espconn_regist_recvcb(&ST_NetCon, ESP8266_WIFI_Recv_callback_ws);    //ע���������ݽ��ճɹ��Ļص�����

	espconn_create(&ST_NetCon);    //��ʼ��UDPͨ��

	espconn_send(&ST_NetCon, "Hello From ESP8266\n", \
			os_strlen("Hello From ESP8266\n"));    //������UDP��������������
}


// �����ʱ�Ļص�����
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// �ڣ�����ص�����
{
	struct ip_info ST_ESP8266_IP;	// IP��Ϣ�ṹ��

	u8  ESP8266_IP[4];		// ���ʮ������ʽ����IP

	wifi_get_ip_info(SOFTAP_IF, &ST_ESP8266_IP);    //��ѯAPģʽ��ESP8266��IP��ַ

	if(ST_ESP8266_IP.ip.addr != 0)
	{
		ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;    //���ʮ���Ƶĵ�һ����
		ESP8266_IP[1] = ST_ESP8266_IP.ip.addr >> 8;    //���ʮ���Ƶĵڶ�����
		ESP8266_IP[2] = ST_ESP8266_IP.ip.addr >> 16;    //���ʮ���Ƶĵ�������
		ESP8266_IP[3] = ST_ESP8266_IP.ip.addr >> 24;    //���ʮ���Ƶĵ��ĸ���
	}
	//��ӡESP8266��IP��ַ
	os_printf("ESP8266 IP = %d.%d.%d.%d", ESP8266_IP[0], ESP8266_IP[1], ESP8266_IP[2], ESP8266_IP[3]);

	os_timer_disarm(&os_timer_1);    //�رն�ʱ��

	ESP8266_UDP_Init();    //��ʼ��UDP��������

}




//�����ʱ��os_timer��ʼ��������ms����
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

//LED��KEY��ʼ��
void ICACHE_FLASH_ATTR LED_Init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);    //����GPIO12Ϊͨ��IO,green
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);    //����GPIO13Ϊͨ��IO,blue
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);    //����GPIO15Ϊͨ��IO,red
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO12);    //����GPIO4Ϊͨ��IO,key

    //����Ϊ���ģʽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //��GPIO12����Ϊ���ģʽ��������͵�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);    //��GPIO13����Ϊ���ģʽ��������͵�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //��GPIO15����Ϊ���ģʽ��������ߵ�ƽ
    //����Ϊ����ģʽ
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(4));			//��GPIO4����Ϊ����ģʽ��Ĭ��Ϊ����״̬

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

	LED_Init();    //��ʼ��LED��KEY

	ESP8266_AP_Init_ws();    //��ʼ��ESP8266AP����
	os_timer_1_Init(30000, 0);    //os_timer1��ʼ����30�룬���Σ�Ϊ�˸�����������С���׼��ʱ��

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

