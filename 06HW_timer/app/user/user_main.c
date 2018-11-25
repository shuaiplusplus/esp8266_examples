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
#define		ProjectName		"HW_timer"		// �������궨��
//==================================================================================


// ȫ�ֱ������˴�����ȫ�ֱ�����
//==================================================================================
bool F_LED = 0;				// LED״̬��־λ
// ע��OS_timer_1���붨��Ϊȫ�ֱ�������ΪESP8266���ں˻�Ҫʹ��
os_timer_t os_timer_1;	// �٣����������ʱ��(os_timer_t�ͽṹ��)
//==================================================================================


// ������ʱ����
//===========================================
void ICACHE_FLASH_ATTR delay_ms(u32 C_time)
{	for(;C_time>0;C_time--)
		os_delay_us(1000);
}
//===========================================



//IO��ʼ������
//RED GPIO15;GREEN GPIO12;BLUE GPIO13;KEY GPIO4
void ICACHE_FLASH_ATTR LED_Init(void)
{
    //�ܽŹ���ѡ������Ϊͨ��IO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U ,FUNC_GPIO12);    //����GPIO12Ϊͨ��IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U ,FUNC_GPIO13);    //����GPIO13Ϊͨ��IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U ,FUNC_GPIO15);    //����GPIO15Ϊͨ��IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U ,FUNC_GPIO4);    //����GPIO4Ϊͨ��IO
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO4_U);    				//����GPIO4����������Ϊ�ⲿ�Ѿ��������ߵ�ƽ
    //����Ϊ���ģʽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);    //��GPIO12����Ϊ���ģʽ��������͵�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);    //��GPIO13����Ϊ���ģʽ��������͵�ƽ
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);    //��GPIO15����Ϊ���ģʽ��������ߵ�ƽ
    //����Ϊ����ģʽ
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(4));			//��GPIO4����Ϊ����ģʽ��Ĭ��Ϊ����״̬
}


//�ⲿ�жϷ������ע�ⲻҪ��ICACHE_FLASH_ATTR��
//======================================================================
void gpio_intr_handler(void)
{
    u32 gpio_status = 0;
    os_printf("enter exti intr\r\n");						  //�����жϴ�ӡһ���������ж�
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);         //���жϷ�����IO�ҳ���
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);    //���ж�
    if(gpio_status & (BIT(4)))    //�ж��ǲ���GPIO4��Ӧ���½����ж�,bit(4)= 10000b��1<<4
    {
		os_printf("key pressed\r\n");    //��ӡһ�������ж��Ƿ��ǰ���������
    	F_LED = !F_LED;    //ȡ��״̬
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED״̬��ת
    }
}
//======================================================================


//�ⲿ�жϳ�ʼ������
void ICACHE_FLASH_ATTR EXTI_init(void)
{
	ETS_GPIO_INTR_DISABLE();    //��ֹ����IO�ж�
	ETS_GPIO_INTR_ATTACH((ets_isr_t)gpio_intr_handler,NULL);   //ע���жϷ�����
	gpio_pin_intr_state_set(GPIO_ID_PIN(4),GPIO_PIN_INTR_NEGEDGE);    //����Ϊ�½��ش�����ʽ
	ETS_GPIO_INTR_ENABLE();    //ʹ��GPIO�ж�
}



// �����ʱ�Ļص�����
//======================================================================
void ICACHE_FLASH_ATTR os_timer_1_callback(void)		// �ڣ�����ص�����
{
	F_LED = !F_LED;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);		// green LED״̬��ת
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13),F_LED);		// blue  LED״̬��ת
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED״̬��ת


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

//Ӳ����ʱ���ص�����
void hw_timer_intr_callback(void)
{
	os_printf("hw_timer in\r\n");    //��ӡһ�������ж��Ƿ��ǰ���������
	F_LED = !F_LED;    //ȡ��״̬
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15),F_LED);		// red   LED״̬��ת
}
/* Ӳ����ʱ����ʼ�����ú�����������һ�������������õ�hw_timer_Init()����
 * ʹ��ע�⣺
 * 1.�����NMI�ж�Դ����Ϊ�Զ���װ�Ķ�ʱ��������hw_timer_armʱ����val�������100
 * 2.���ʹ��NMI�ж�Դ����ô��ʱ����Ϊ������ȼ����ɴ������ISR
 * 3.���ʹ��FRC1�ж�Դ����ô�ö�ʱ���޷��������ISR
 * 4.hw_timer.c�Ľӿڲ��ܸ�PWM�����ӿں���ͬʱʹ�ã���Ϊ���߹�����ͬһ��Ӳ����ʱ��
 * 5.ʹ��hw_timer.c�ӿڣ��������wifi_set_sleep_type(LIGHT_SLEEP),��������ӦNMI�ж�
 * 6.ʹ��FRC1�ж�Դ��ȡֵ��Χ50-1677721us��ʹ��NMI�ж�Դ��ȡֵ��Χ��100-1677721us
 */
void ICACHE_FLASH_ATTR hw_timer_Init(void)
{
	//��ʼ��Ӳ��ISR��ʱ��
	//��һ��������FRC1_SOURCE��ʹ��FRC1�ж�Դ��NMI_SOURCE��ʹ��NMI�ж�Դ��
	//�ڶ���������0 ���Զ���װ��1 �Զ���װ
	hw_timer_init(0 ,1);    //ʹ��FRC1�ж�Դ���Զ���װ
	hw_timer_set_func(hw_timer_intr_callback);    //����Ӳ����ʱ���жϻص�����
	hw_timer_arm(500000);    //���ö�ʱʱ��Ϊ500ms
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

	LED_Init();    //LED��ʼ��
	//os_timer1��ʼ����500���룬�ظ�
	//os_timer_1_Init(500, 1);   //��������ò���os_timer������֮
	//EXTI_init();    //�ⲿ�жϳ�ʼ�����������ò���������֮
	hw_timer_Init();    //Ӳ����ʱ����ʼ��
	os_printf("\r\n==================user init end=================\r\n");

	/*while(1)
	{
		system_soft_wdt_feed();    //ι��
	}*/
}

