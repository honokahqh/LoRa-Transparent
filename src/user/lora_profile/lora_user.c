#include "lora_core.h"
#include "lora_user.h"

#include "APP.h"
#include "ymodem.h"

/**
 * ChipID_Syn
 * @brief 芯片数据同步至MAC和chipID
 * @author Honokahqh
 * @date 2023-08-05
 */
void ChipID_Syn()
{
    uint32_t id[2];
    id[0] = EFC->SN_L;
    id[1] = EFC->SN_H;
    Lora_State.Mac[0] = id[0] & 0xFF;
    Lora_State.Mac[1] = (id[0] >> 8) & 0xFF;
    Lora_State.Mac[2] = (id[0] >> 16) & 0xFF;
    Lora_State.Mac[3] = (id[0] >> 24) & 0xFF;

    Lora_State.Mac[4] = id[1] & 0xFF;
    Lora_State.Mac[5] = (id[1] >> 8) & 0xFF;
    Lora_State.Mac[6] = (id[1] >> 16) & 0xFF;
    Lora_State.Mac[7] = (id[1] >> 24) & 0xFF;
    Lora_State.chip_ID = Lora_State.Mac[0] + Lora_State.Mac[1] + Lora_State.Mac[2] + Lora_State.Mac[3] + Lora_State.Mac[4] + Lora_State.Mac[5] + Lora_State.Mac[6] + Lora_State.Mac[7];
}

/**
 * Lora_StateInit
 * @brief 设备初始化
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_StateInit()
{
    Lora_State.DeviceType = Device_Type;
    Lora_State_Data_Syn();
    Lora_AsData_Syn();
}

/**
 * User_Slaver_Cmd
 * @brief 自定义的指令
 * @author Honokahqh
 * @date 2023-08-05
 */
void User_Slaver_Cmd()
{
    switch (Lora_State.Rx_CMD)
    {

    default:
        break;
    }
}

/**
 * Random_Delay
 * @brief 0~10ms的随机延迟
 * @author Honokahqh
 * @date 2023-08-05
 */
void Random_Delay()
{
    static uint8_t i;
    uint16_t ms;
    ms = (Lora_State.Mac[3] << 8) + Lora_State.Mac[4];
    ms = (ms / (1 + i * 2)) % 10;
    i++;
    delay_ms(ms);
}

/**
 * Lora_Send_Data
 * @brief 数据发送
 * @param data 要发送的数据
 * @param len 数据的长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Send_Data(uint8_t *data, uint8_t len)
{
    // Random_Delay();
    delay_ms(10);
    //	while(1)
    //	{
    Radio.Send(data, len);
    Wait2TXEnd();
    //	}
}

/**
 * Lora_Sleep
 * @brief Lora进入睡眠模式
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Sleep()
{
    Radio.Rx(0);
    Wait2RXEnd();
    Radio.Sleep();
    if (Radio.GetStatus() != RF_IDLE)
        return;
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    delay_ms(5);
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    delay_ms(5);
}

void PCmd_LoRa_SetSlaverName(char *name, uint8_t ID)
{
    if (ID >= Device_Num_Max)
        return;
    memset(Associated_devices[ID].Name, 0, sizeof(Associated_devices[ID].Name));
    strncpy(Associated_devices[ID].Name, name, sizeof(Associated_devices[ID].Name) - 1);
    CusProfile_Send(Associated_devices[ID].SAddr, LoRa_SetSlaverName, (uint8_t *)name,
                    strnlen(name, sizeof(Associated_devices[ID].Name) - 1), FALSE);
    Lora_AsData_Add(ID);
}

void Cmd_LoRa_SetSlaverName()
{
    char BleBuffer[50];
    memset(BleBuffer, 0, sizeof(BleBuffer));
    memset(Lora_State.SelfName, 0, sizeof(Lora_State.SelfName));
    strncpy(Lora_State.SelfName, (char *)&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
    Lora_State.SelfName[sizeof(Lora_State.SelfName) - 1] = '\0';
    sprintf(BleBuffer, "AT+NAME=乐清%s\r\n", Lora_State.SelfName);
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    delay_ms(500);
    sprintf(BleBuffer, "AT+REBOOT=1\r\n");
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    Lora_State_Save();
    Lora_State_Data_Syn();
}

void PCmd_LoRa_SetSelfName(char *name)
{
    memset(Lora_State.SelfName, 0, sizeof(Lora_State.SelfName));
    strncpy(Lora_State.SelfName, name, sizeof(Lora_State.SelfName) - 1);
    CusProfile_Send(BoardCast, LoRa_SetSelfName, (uint8_t *)Lora_State.SelfName,
                    strnlen(Lora_State.SelfName, sizeof(Lora_State.SelfName) - 1), FALSE);
    Lora_State_Save();
}

void Cmd_LoRa_SetSelfName()
{
    if (Lora_State.Rx_SAddr == Lora_State.SAddrMaster)
    {
        memset(Lora_State.MasterName, 0, sizeof(Lora_State.MasterName));
        strncpy(Lora_State.MasterName, (char *)&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
        Lora_State.MasterName[sizeof(Lora_State.MasterName) - 1] = '\0';
        Lora_State_Save();
    }
    else
    {
        for (uint8_t i = 0; i < Device_Num_Max; i++)
        {
            if (Associated_devices[i].SAddr == Lora_State.Rx_SAddr)
            {
                memset(Associated_devices[i].Name, 0, sizeof(Associated_devices[i].Name));
                strncpy(Associated_devices[i].Name, (char *)&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
                Associated_devices[i].Name[sizeof(Associated_devices[i].Name) - 1] = '\0';
                Lora_AsData_Add(i);
                break;
            }
        }
    }
}

void PCmd_Lora_Find_Relay(char *name)
{
    char name_buffer[Name_Size * 2 + 2];
    memset(name_buffer, 0, sizeof(name_buffer));
    if (strlen(name) > sizeof(Lora_State.RelayName) - 1)
        return;
    if (strlen(name) > sizeof(Lora_State.SelfName) - 1)
        return;
    sprintf(name_buffer, "%s__%s", Lora_State.RelayName, Lora_State.SelfName);
    CusProfile_Send(BoardCast, Lora_Find_Relay, name_buffer, strlen(name_buffer), FALSE);
}

void Cmd_Lora_Find_Relay()
{
    if (Lora_State.Rx_SAddr == Lora_State.SAddrMaster)
    {
        memset(Lora_State.RelayName, 0, sizeof(Lora_State.RelayName));
        strncpy(Lora_State.RelayName, (char *)&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
        Lora_State.RelayName[sizeof(Lora_State.RelayName) - 1] = '\0';
        Lora_State_Save();
    }
}
