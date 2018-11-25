by WangShuai
2018年11月24日
此工程测试系统的GPIO输入，采用一个按键来测试(非中断模式)。
【包含内容】
1.GPIO输入（按键消抖），输入状态是默认的
2.GPIO输出
3.os_timer定时器
4.os_printf()
5.喂狗
6.毫秒/微秒延时函数
需要注意的一点：
PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
函数中，第一个参数是功能选择寄存器，不同的GPIO，该寄存器不同。
第二个参数FUNC_GPIO12=3，在EXCEL表中有对应的不同功能，其中GPIO在第4个，从0开始，这里为3。