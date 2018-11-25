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
#define		ProjectName		"delay_Test"		// �������궨��
//==================================================================================


// ȫ�ֱ������˴�����ȫ�ֱ�����
//==================================================================================
bool F_LED = 0;				// LED״̬��־λ
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



//LED��ʼ������
//RED GPIO15;GREEN GPIO12;BLUE GPIO13.
void ICACHE_FLASH_ATTR LED_Init(void)
{
    //�ܽŹ���ѡ������Ϊͨ��IO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U ,FUNC_GPIO12);    //����GPIO12Ϊͨ��IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U ,FUNC_GPIO13);    //����GPIO13Ϊͨ��IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U ,FUNC_GPIO15);    //����GPIO15Ϊͨ��IO
    //����Ϊ���ģʽ��������ߵ�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);    //��GPIO12����Ϊ���ģʽ��������ߵ�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);    //��GPIO12����Ϊ���ģʽ��������ߵ�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //��GPIO12����Ϊ���ģʽ��������ߵ�ƽ
}


// �����ʱ�Ļص�����
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// �ڣ�����ص�����
{
	F_LED = !F_LED;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED״̬��ת
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),F_LED);		// blue  LED״̬��ת
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),!F_LED);		// red   LED״̬��ת


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
    uart_init(115200,115200);    //��ʼ�����ڲ�����
    os_delay_us(10000);          //�ȴ������ȶ�
	os_printf("\r\n=================================================\r\n");
	os_printf("\t Project:\t%s\r\n", ProjectName);
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	LED_Init();
	//os_timer1��ʼ����500����
	//os_timer_1_Init(500, 1);    //delay_Test�ò�����ע�͵�

	os_printf("\r\n==================user init end=================\r\n");

	while(1)    //ֻ��������ɿ��Ź���λ��ֻ��while(1)��
	{
		system_soft_wdt_feed(); //�����Ͳ�����ɿ��Ź���λ�����ǻᵼ�³�������Ӧ��ʱ��os_timer�Ļص������while(1)����ֻ����һ�䣩
		F_LED = !F_LED;
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED״̬��ת
		os_printf("hello world\r\n");    //���hello world
		delay_ms(1000);
	}
}


