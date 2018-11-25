by WangShuai
2018年11月24日
此工程测试系统delay函数，在watchdog_Test基础上更改
注意：

        F_LED = !F_LED;
        GPIO_OUTPUT_SET(GPIO_ID_PIN(12),F_LED);     // green LED状态翻转
此处不能直接写成GPIO_OUTPUT_SET(GPIO_ID_PIN(12),!F_LED);     // green LED状态翻转
不会执行反转。