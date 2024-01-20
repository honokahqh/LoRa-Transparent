#ifndef __APP_H
#define __APP_H

#include <stdio.h>

#include "lora_core.h"
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "tremo_delay.h"
#include "tremo_bstimer.h"
#include "tremo_system.h"
#include "tremo_flash.h"
#include "tremo_adc.h"
#include "tremo_timer.h"
#include "tremo_lptimer.h"
#include "tremo_regs.h"
#include "tremo_wdg.h"
#include "tremo_dac.h"
#include "pt.h"
#include "log.h"

#define Dev_Version 101
//#define __DEBUG
#ifdef __DEBUG
#define normal_printf(format, ...) printf(format, ##__VA_ARGS__);
#define tick_printf(format, ...)                                   \
    do                                                             \
    {                                                              \
        printf("(%u) ", millis());               \
        printf(format, ##__VA_ARGS__);                             \
    } while (0)
#define normal_printf(format, ...) printf(format, ##__VA_ARGS__);

#else
#define tick_printf(format, ...)
#define normal_printf(format, ...)

#endif

#ifdef __DEBUG
#define Uart_BAUD 115200
#else
#define Uart_BAUD 115200
#endif

#define UART_RX_PORT GPIOB
#define UART_RX_PIN GPIO_PIN_0
#define UART_TX_PORT GPIOB
#define UART_TX_PIN GPIO_PIN_1

#define UART1_RX_PORT GPIOA
#define UART1_RX_PIN GPIO_PIN_4
#define UART1_TX_PORT GPIOA
#define UART1_TX_PIN GPIO_PIN_5
/* uart状态 */
#define UART_IDLE_Timeout 5
typedef struct
{
    volatile uint8_t busy;     // 串口忙
    volatile uint8_t IDLE;     // 串口空闲-0:空闲
    volatile uint8_t has_data; // 串口一帧数据接收完成
    volatile uint8_t connect;  // ble connect
    uint8_t rx_buff[257];      // 保证最后一个字节为0
    uint8_t rx_len;
} uart_state_t;
extern uart_state_t uart_state, ble_state;

void uart_log_init(uint32_t uart0_baud);
void System_Run(void);
void lora_init(void);
void LoraReInit(void);
void Lora_IRQ_Rrocess(void);

void Flash_Data_Syn();
void LoraState_Save();
void UART_SendData(const uint8_t *data, uint16_t len);
void UART1_SendData(const uint8_t *data, uint16_t len);
void BLE_SendData(const uint8_t *data, uint16_t len);
extern uint32_t Sys_ms;

#endif
