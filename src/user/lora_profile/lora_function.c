#include "lora_core.h"

#define TAG "LoRa function"
/**
 * Get_IDLE_ID
 * @brief 获取最小空闲ID
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_IDLE_ID()
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress == 0)
            break;
    }
    if (i < MAX_CHILDREN) // 有空闲ID
        return i;
    return 0xFF;
}

/**
 * Compare_ShortAddr
 * @brief 短地址匹配
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_ShortAddr(uint16_t Short_Addr)
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
        if (LoRaDevice.Slaver[i].shortAddress == Short_Addr && LoRaDevice.Slaver[i].shortAddress != 0)
            break;
    if (i < MAX_CHILDREN)
        return i;
    return 0xFF;
}

/**
 * Compare_MAC
 * @brief MAC地址匹配
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_MAC(uint8_t *Mac)
{
    uint8_t i, j, count;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        count = 0;
        for (j = 0; j < MAC_Size; j++)
        {
            if (LoRaDevice.Slaver[i].Mac[j] == Mac[j])
                count++;
        }
        if (count == MAC_Size)
            return i;
    }
    return 0xFF;
}

/**
 * XOR_Calculate
 * @brief 异或校验计算
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t XOR_Calculate(uint8_t *data, uint8_t len)
{
    uint8_t x_or = 0, i;
    for (i = 0; i < len; i++)
        x_or = x_or ^ *data++;
    return x_or;
}

/**
 * Lora_ReceiveData2State
 * @brief 接收数据存入各类参数
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_ReceiveData2State()
{
    LoRaPacket.Rx_DevType = LoRaPacket.Rx_Data[DevType_Addr];
    LoRaPacket.Rx_PanID = (LoRaPacket.Rx_Data[PanIDH_Addr] << 8) + LoRaPacket.Rx_Data[PanIDL_Addr];
    LoRaPacket.Rx_DAddr = (LoRaPacket.Rx_Data[DAddrH_Addr] << 8) + LoRaPacket.Rx_Data[DAddrL_Addr];
    LoRaPacket.Rx_SAddr = (LoRaPacket.Rx_Data[SAddrH_Addr] << 8) + LoRaPacket.Rx_Data[SAddrL_Addr];
    LoRaPacket.Rx_CMD = LoRaPacket.Rx_Data[Cmd_Addr];
    LoRaPacket.Rx_PID = LoRaPacket.Rx_Data[PackID_Addr];
    LOG_I(TAG, "receive packet from PanID:%04X, SAddr:%04X, DAddr:%04X, CMD:%02X, data:",
          LoRaPacket.Rx_PanID, LoRaPacket.Rx_SAddr, LoRaPacket.Rx_DAddr, LoRaPacket.Rx_CMD);
#if LOG_LEVEL >= LOG_INFO
    // for (uint8_t i = 0; i < LoRaPacket.Rx_Len; i++)
    //     printf("%02X ", LoRaPacket.Rx_Data[i]); // 打印全部数据
    for (uint8_t i = 0; i < LoRaPacket.Rx_Data[Len_Addr]; i++)
        printf("%02X ", LoRaPacket.Rx_Data[Data_Addr + i]);  // 仅打印数据部分
    printf("\r\n");
#endif
}

int StringToMac(const char *str, uint8_t Mac[8])
{
    // 检查输入字符串是否为 NULL    
    if (str == NULL)
    {
        return 0;
    }

    // 检查输入字符串的格式
    for (int i = 0; i < 23; i++)
    {
        if (i % 3 == 2)
        {
            if (str[i] != ':')
            {
                return 0; // 格式错误
            }
        }
        else
        {
            if (!((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F')))
            {
                return 0; // 格式错误
            }
            else
            {
                if (i % 3 == 0)
                {
                    if (str[i] >= '0' && str[i] <= '9')
                    {
                        Mac[i / 3] = (str[i] - '0') << 4;
                    }
                    else if (str[i] >= 'a' && str[i] <= 'f')
                    {
                        Mac[i / 3] = (str[i] - 'a' + 10) << 4;
                    }
                    else if (str[i] >= 'A' && str[i] <= 'F')
                    {
                        Mac[i / 3] = (str[i] - 'A' + 10) << 4;
                    }
                }
                else
                {
                    if (str[i] >= '0' && str[i] <= '9')
                    {
                        Mac[i / 3] |= (str[i] - '0');
                    }
                    else if (str[i] >= 'a' && str[i] <= 'f')
                    {
                        Mac[i / 3] |= (str[i] - 'a' + 10);
                    }
                    else if (str[i] >= 'A' && str[i] <= 'F')
                    {
                        Mac[i / 3] |= (str[i] - 'A' + 10);
                    }
                }
            }
        }
    }
    // 检查字符串的结束
    if (str[23] != '\0')
    {
        return 0; // 格式错误
    }
    return 1; // 成功转换
}

// 函数将 uint8_t 数组转换为冒号分隔的字符串
void MacToString(const uint8_t Mac[8], char *str)
{
    for(uint8_t i = 0;i < 8;i++)
    {
        if((Mac[i] >> 4) < 10)
            str[i*3 + 0] = (Mac[i] >> 4) + '0';
        else
            str[i*3 + 0] = (Mac[i] >> 4) - 10 + 'A';
        if((Mac[i] & 0x0F) < 10)
            str[i*3 + 1] = (Mac[i] & 0x0F) + '0';
        else
            str[i*3 + 1] = (Mac[i] & 0x0F) - 10 + 'A';
        str[i*3 + 2] = ':'; 
    }
    str[23] = '\0';
}

// 函数比较两个 MAC 地址是否相同
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8])
{
    return memcmp(Mac1, Mac2, 8) == 0;
}

void Reset_LoRa()
{
    LoRaFlashdataSyn();
    LoraReInit();
    // system_reset();
}

