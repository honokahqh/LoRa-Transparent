#include "APP.h"

static struct pt Lora_Rx;
static struct pt Period_100ms;
static struct pt uart1;
static struct pt uart0;


static int Task1_Lora_Rx(struct pt *pt);
static int Task2_100ms_Period(struct pt *pt);
static int Task3_uart1(struct pt *pt);
static int Task4_uart0(struct pt *pt);

/**
 * System_Run
 * @brief PTOS运行
 * @author Honokahqh
 * @date 2023-08-05
 */
void System_Run()
{
	PT_INIT(&Lora_Rx);
	PT_INIT(&Period_100ms);
	PT_INIT(&uart1);
	PT_INIT(&uart0);
	while (1)
	{
		Task1_Lora_Rx(&Lora_Rx);
		Task2_100ms_Period(&Period_100ms);
		Task3_uart1(&uart1);
		Task4_uart0(&uart0);
	}
}

/**
 * Task1_Lora_Rx
 * @brief Lora中断处理、Rst按键处理
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task1_Lora_Rx(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		Lora_IRQ_Rrocess();
		PT_TIMER_DELAY(pt, 5);
	}
	PT_END(pt);
}

/**
 * Task2_100ms_Period
 * @brief 1秒周期运行-lora slaver状态更新-复位检测-led闪烁-喂狗
 * @author Honokahqh
 * @date 2023-08-05
 */
uint32_t count_100ms;
static int Task2_100ms_Period(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		if(LoRaDevice.NetMode)
		{
			if(count_100ms % 5 == 0)
				gpio_toggle(GPIOD, GPIO_PIN_12);
		}
		else
		{
			if(count_100ms % 10 == 0)
				gpio_toggle(GPIOD, GPIO_PIN_12);
		}
		
		if(count_100ms % 10 == 0)
		{
			LoRa_Period_1s();
			wdg_reload();				// 喂狗
		}
		count_100ms++;
		PT_TIMER_DELAY(pt, 100);
	}
	PT_END(pt);
}

/**
 * Task3_uart1
 * @brief 串口1 用于和蓝牙模块通讯
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task3_uart1(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt, ble_state.has_data); // 等待串口数据
		processATCommand((char *)ble_state.rx_buff);
		memset(ble_state.rx_buff, 0, sizeof(ble_state.rx_buff));
		ble_state.has_data = 0;
		ble_state.rx_len = 0;
	}
	PT_END(pt);
}

/**
 * Task4_uart0
 * @brief mbs主机轮询-中控外设控制-数据保存
 * @author Honokahqh
 * @date 2023-08-05
 */	
static int Task4_uart0(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		PT_WAIT_UNTIL(pt, uart_state.has_data); // 等待串口数据
		if(uart_state.rx_len && LoRaDevice.PanID != BoardCast)
		{
			CusProfile_Send(BoardCast, Lora_SendData, uart_state.rx_buff, uart_state.rx_len, false);
		}
		memset(uart_state.rx_buff, 0, sizeof(uart_state.rx_buff));
		uart_state.rx_len = 0;
		uart_state.has_data = 0;
	}
	PT_END(pt);
}
