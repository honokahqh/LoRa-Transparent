#include <stdio.h>
#include <string.h>
#include "APP.h"

#define TAG "main" 

static void BSTimer_Init(void);
static void watch_dog_init(void);
static void board_init(void);

uint32_t Sys_ms;
uart_state_t uart_state, ble_state;
const char *prefix = "+NAME:乐清";
char BleBuffer[50] = {0};
int main(void)
{
    uint8_t BleRxTimeout = 50;
    board_init();                                // 硬件初始化
    Lora_StateInit();                            // 状态初始化
    delay_ms(((LoRaDevice.chip_ID) % 10) * 100); // 随机延迟
    lora_init();                                 // lora初始化
    wdg_reload();                                // 看门狗喂狗
    sprintf(BleBuffer, "AT+NAME?\r\n");
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    sprintf(BleBuffer, "%s%02X%02X\r\n", prefix, LoRaDevice.chip_ID >> 8, LoRaDevice.chip_ID & 0xFF);
    while (BleRxTimeout--)
    {
        if (ble_state.has_data)
        {
            sprintf(BleBuffer, "+NAME:乐清%s", LoRaDevice.Self.name);
            if (strncmp(ble_state.rx_buff, BleBuffer, strlen(BleBuffer)) == 0)
            {
                LOG_I(TAG, "ble receive:%s\r\n", ble_state.rx_buff);
                break;
            }
            else
            {
                delay_ms(100);
                LOG_W(TAG, "ble name error, receive:%s\r\n", ble_state.rx_buff);
                sprintf(BleBuffer, "AT+NAME=乐清%s\r\n", LoRaDevice.Self.name);
                UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
                delay_ms(500);
                sprintf(BleBuffer, "AT+REBOOT=1\r\n");
                UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
                break;
            }
        }
        delay_ms(100);
        wdg_reload();
    }
    System_Run();
}
void LoraReInit()
{
    /* 复位Lora模块 */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
    delay_ms(10);
    lora_init(); // lora初始化
}

/**
 * BSTimer_Init
 * @brief 定时器初始化,1ms定时
 * @author Honokahqh
 * @date 2023-08-05
 */
static void BSTimer_Init()
{
    bstimer_init_t bstimer_init_config;

    bstimer_init_config.bstimer_mms = BSTIMER_MMS_ENABLE;
    bstimer_init_config.period = 23;     // time period is ((1 / 2.4k) * (2399 + 1))
    bstimer_init_config.prescaler = 999; // sysclock defaults to 24M, is divided by (prescaler + 1) to 2.4k
    bstimer_init_config.autoreload_preload = true;
    bstimer_init(BSTIMER0, &bstimer_init_config);
    bstimer_config_overflow_update(BSTIMER0, ENABLE);
    bstimer_config_interrupt(BSTIMER0, ENABLE);
    bstimer_cmd(BSTIMER0, true);
    NVIC_EnableIRQ(BSTIMER0_IRQn);
    NVIC_SetPriority(BSTIMER0_IRQn, 2);
}

/**
 * BSTIMER0_IRQHandler
 * @brief 定时器中断服务函数,1ms定时,用于计时和485接收超时判断,485接收超时后,将数据发送给处理函数
 * @author Honokahqh
 * @date 2023-08-05
 */
void BSTIMER0_IRQHandler(void)
{
    if (bstimer_get_status(BSTIMER0, BSTIMER_SR_UIF))
    {
        // UIF flag is active
        Sys_ms++;
        if (uart_state.busy)
        {
            uart_state.IDLE++;
            if (uart_state.IDLE >= UART_IDLE_Timeout)
            {
                uart_state.busy = 0;
                uart_state.IDLE = 0;
                uart_state.has_data = true;
            }
        }
        if (ble_state.busy)
        {
            ble_state.IDLE++;
            if (ble_state.IDLE >= 100) // 蓝牙的串口发送会断帧
            {
                ble_state.busy = 0;
                ble_state.IDLE = 0;
                ble_state.has_data = true;
            }
        }
    }
}

/**
 * millis
 * @brief ptos会调用,用于OS系统
 * @return 系统时间
 * @author Honokahqh
 * @date 2023-08-05
 */
unsigned int millis(void)
{
    return Sys_ms;
}

/**
 * UART_SendData
 * @brief 串口0发送数据
 * @param data 数据指针
 * @param len 数据长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART_SendData(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_FULL) == SET)
            ;
        UART0->DR = *data++;
    }
    while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) == RESET)
        ;
}

/**
 * UART0_IRQHandler
 * @brief 接收数据传入mbs缓冲区,并设置接收标志位,由timer0进行IDLE计时
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART0_IRQHandler(void)
{
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
        uart_state.rx_buff[uart_state.rx_len++] = UART0->DR & 0xFF;
        uart_state.busy = true;
        uart_state.IDLE = 0;
    }
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_TIMEOUT))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT);
    }
}

void UART1_SendData(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        while (uart_get_flag_status(UART1, UART_FLAG_TX_FIFO_FULL) == SET)
            ;
        UART1->DR = *data++;
    }
    while (uart_get_flag_status(UART1, UART_FLAG_TX_FIFO_EMPTY) == RESET)
        ;
}
void BLE_SendData(const uint8_t *data, uint16_t len)
{
    if (ble_state.connect)
    {
        LOG_I(TAG, "blesend:%s", data);
        UART1_SendData(data, len);
        delay_ms(100);
    }
}
void UART1_IRQHandler(void)
{
    if (uart_get_interrupt_status(UART1, UART_INTERRUPT_RX_DONE))
    {
        uart_clear_interrupt(UART1, UART_INTERRUPT_RX_DONE);
        ble_state.rx_buff[ble_state.rx_len++] = UART1->DR & 0xFF;
        ble_state.busy = true;
        ble_state.IDLE = 0;
    }
    if (uart_get_interrupt_status(UART1, UART_INTERRUPT_RX_TIMEOUT))
    {
        uart_clear_interrupt(UART1, UART_INTERRUPT_RX_TIMEOUT);
    }
}
/**
 * uart_log_init
 * @brief IO16\IO17初始化为UART0,工作模式:9600、Debug:1M
 * @author Honokahqh
 * @date 2023-08-05
 */
void uart_log_init(uint32_t uart0_baud)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART1, true);

    gpio_set_iomux(UART_TX_PORT, UART_TX_PIN, 1); // TX
    gpio_set_iomux(UART_RX_PORT, UART_RX_PIN, 1); // RX

    gpio_set_iomux(UART1_TX_PORT, UART1_TX_PIN, 2); // TX
    gpio_set_iomux(UART1_RX_PORT, UART1_RX_PIN, 2); // RX
    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = uart0_baud;
    uart_init(UART0, &uart_config);
    uart_config.baudrate = 115200;
    uart_init(UART1, &uart_config);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_DONE, ENABLE);
    uart_config_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT, ENABLE);
    uart_config_interrupt(UART1, UART_INTERRUPT_RX_DONE, ENABLE);
    uart_config_interrupt(UART1, UART_INTERRUPT_RX_TIMEOUT, ENABLE);

    uart_cmd(UART0, ENABLE);
    uart_cmd(UART1, ENABLE);
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART1_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2);
    NVIC_SetPriority(UART1_IRQn, 2);
}

/**
 * watch_dog_init
 * @brief 看门口初始化:10s
 * @author Honokahqh
 * @date 2023-08-05
 */
static void watch_dog_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_WDG, true);
    uint32_t timeout = 10000;
    uint32_t wdgclk_freq = rcc_get_clk_freq(RCC_PCLK0);
    uint32_t reload_value = timeout * (wdgclk_freq / 1000 / 2);

    // start wdg
    wdg_start(reload_value);
    NVIC_EnableIRQ(WDG_IRQn);
}

/**
 * board_init
 * @brief 硬件初始化
 * @author Honokahqh
 * @date 2023-08-05
 */
static void board_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_AFEC, true);

    // enable the clk
    rcc_enable_oscillator(RCC_OSC_XO32K, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER1, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER1, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    delay_ms(100);

//    watch_dog_init();
    pwr_xo32k_lpm_cmd(true);
    BSTimer_Init();
    /* 复位Lora模块 */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
    // BRTS LOW
    gpio_init(GPIOC, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_write(GPIOC, GPIO_PIN_15, GPIO_LEVEL_LOW);
    gpio_init(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_write(GPIOA, GPIO_PIN_8, GPIO_LEVEL_LOW);
    gpio_init(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_write(GPIOD, GPIO_PIN_14, GPIO_LEVEL_LOW);
    // LED
    gpio_init(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_write(GPIOD, GPIO_PIN_12, GPIO_LEVEL_LOW);
}

#ifdef USE_FULL_ASSERT
void assert_failed(void *file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1)
    {
    }
}
#endif
