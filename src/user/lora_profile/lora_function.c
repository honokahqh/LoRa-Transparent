#include "lora_core.h"

/**
 * Get_IDLE_ID
 * @brief ��ȡ��С����ID
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_IDLE_ID()
{
    uint8_t i;
    for (i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr == 0)
            break;
    }
    if (i < Device_Num_Max) // �п���ID
        return i;
    return 0xFF;
}

/**
 * Compare_ShortAddr
 * @brief �̵�ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_ShortAddr(uint16_t Short_Addr)
{
    uint8_t i;
    for (i = 0; i < Device_Num_Max; i++)
        if (Associated_devices[i].SAddr == Short_Addr && Associated_devices[i].SAddr != 0)
            break;
    if (i < Device_Num_Max)
        return i;
    return 0xFF;
}
uint8_t Compare_Register_SAddr(uint16_t Short_Addr)
{
    return 0;
}
/**
 * Compare_MAC
 * @brief MAC��ַƥ��
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_MAC(uint8_t *Mac)
{
    uint8_t i, j, count;
    for (i = 0; i < Device_Num_Max; i++)
    {
        count = 0;
        for (j = 0; j < 8; j++)
        {
            if (Associated_devices[i].Mac[j] == Mac[j])
                count++;
        }
        if (count == 8)
            return i;
    }
    return 0xFF;
}

/**
 * XOR_Calculate
 * @brief ���У�����
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
 * @brief �������ݴ���������
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_ReceiveData2State()
{
    Lora_State.Rx_DevType = Lora_State.Rx_Data[DevType_Addr];
    Lora_State.Rx_PanID = (Lora_State.Rx_Data[PanIDH_Addr] << 8) + Lora_State.Rx_Data[PanIDL_Addr];
    Lora_State.Rx_DAddr = (Lora_State.Rx_Data[DAddrH_Addr] << 8) + Lora_State.Rx_Data[DAddrL_Addr];
    Lora_State.Rx_SAddr = (Lora_State.Rx_Data[SAddrH_Addr] << 8) + Lora_State.Rx_Data[SAddrL_Addr];
    Lora_State.Rx_CMD = Lora_State.Rx_Data[Cmd_Addr];
    Lora_State.Rx_PID = Lora_State.Rx_Data[PackID_Addr];
}

int StringToMac(const char *str, uint8_t Mac[8])
{
    // ��������ַ����Ƿ�Ϊ NULL    
    if (str == NULL)
    {
        return 0;
    }

    // ��������ַ����ĸ�ʽ
    for (int i = 0; i < 23; i++)
    {
        if (i % 3 == 2)
        {
            if (str[i] != ':')
            {
                return 0; // ��ʽ����
            }
        }
        else
        {
            if (!((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F')))
            {
                return 0; // ��ʽ����
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
    // ����ַ����Ľ���
    if (str[23] != '\0')
    {
        return 0; // ��ʽ����
    }
    return 1; // �ɹ�ת��
}

// ������ uint8_t ����ת��Ϊð�ŷָ����ַ���
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

// �����Ƚ����� MAC ��ַ�Ƿ���ͬ
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8])
{
    return memcmp(Mac1, Mac2, 8) == 0;
}
