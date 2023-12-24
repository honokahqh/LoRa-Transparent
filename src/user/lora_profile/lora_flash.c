/* 主要函数 flash_program_bytes(uint32_t addr, uint8_t* data, uint32_t size)
            flash_erase_page(uint32_t addr) */
/* FlashData1_ADDR 网络参数:短地址、PANID、入网标志,单页4KB，最小8字节写入 512次 */
/* OTA_ADDR IAP参数:APP1(新版本固件vX.x) APP2(新版本固件vX.x) */

#include "Lora_core.h"

uint16_t page1_offset, page2_offset, page3_offset;
// page 1 存储类型
#define Type_Lora_net 0x01
#define Type_Lora_net_len 16

#define Type_Lora_SelfName 0x02
#define Type_Lora_SelfName_len Name_Size

#define Type_Lora_MasterName 0x03
#define Type_Lora_MasterName_len Name_Size

#define Type_Lora_Relay 0x04
#define Type_Lora_Relay_len Name_Size + 2

void lora_save_data(uint8_t type)
{
    uint8_t temp_data[24];
    memset(temp_data, 0, 24);
    if(page1_offset >= 500)
    {
        page1_offset = 0;
        flash_erase_page(FlashData1_ADDR);
        lora_save_data(Type_Lora_net);
        lora_save_data(Type_Lora_SelfName);
        lora_save_data(Type_Lora_MasterName);
        lora_save_data(Type_Lora_Relay);
    }
    switch (type)
    {
    case 1:
        /* code */
        temp_data[0] = type;
        temp_data[1] = Lora_Para_AT.SpreadingFactor & 0x0F;
        temp_data[2] = Lora_Para_AT.BandWidth & 0x03;
        temp_data[3] = Lora_Para_AT.NetOpen;
        temp_data[4] = Lora_Para_AT.Net_State;
        temp_data[5] = Lora_Para_AT.channel & 0x7F;
        temp_data[6] = Lora_Para_AT.SAddrSelf >> 8;
        temp_data[7] = Lora_Para_AT.SAddrSelf;
        temp_data[8] = Lora_Para_AT.SAddrMaster >> 8;
        temp_data[9] = Lora_Para_AT.SAddrMaster;
        temp_data[10] = Lora_Para_AT.PanID >> 8;
        temp_data[11] = Lora_Para_AT.PanID;
        temp_data[12] = Lora_Para_AT.UART_BAUD >> 24;
        temp_data[13] = Lora_Para_AT.UART_BAUD >> 16;
        temp_data[14] = Lora_Para_AT.UART_BAUD >> 8;
        temp_data[15] = Lora_Para_AT.UART_BAUD;
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, 16);
        page1_offset += (Type_Lora_net_len + 7) / 8;
        break;
    case 2:
        temp_data[0] = type;
        memcpy(&temp_data[1], Lora_State.SelfName, Name_Size - 1);
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Name_Size);
        page1_offset += (Type_Lora_SelfName_len + 7) / 8;
        break;
    case 3:
        temp_data[0] = type;
        memcpy(&temp_data[1], Lora_State.MasterName, Name_Size - 1);
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Name_Size);
        page1_offset += (Type_Lora_MasterName_len + 7) / 8;
        break;
    case 4:
        temp_data[0] = type;
        memcpy(&temp_data[1], Lora_State.RelayName, Name_Size - 1);
        temp_data[Name_Size] = Lora_State.RelaySAddr >> 8;
        temp_data[Name_Size + 1] = Lora_State.RelaySAddr;
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Name_Size + 2);
        page1_offset += (Type_Lora_Relay_len + 7) / 8;
        break;
    default:
        break;
    }
}

/**
 * Lora_State_Data_Syn
 * @brief Lora网络数据读取
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_State_Data_Syn()
{
    uint32_t data;
    ChipID_Syn();
    // 先设置为默认值
    Lora_Para_AT.SpreadingFactor = 7;
    Lora_Para_AT.BandWidth = 2;
    Lora_Para_AT.NetOpen = 0;
    Lora_Para_AT.Net_State = 0;
    Lora_Para_AT.channel = 0;
    Lora_Para_AT.SAddrSelf = Lora_State.chip_ID;
    Lora_Para_AT.SAddrMaster = 0;
    Lora_Para_AT.PanID = Lora_State.chip_ID;
    Lora_Para_AT.UART_BAUD = 115200;
    sprintf(Lora_State.SelfName, "ID:%04X", Lora_State.chip_ID);
    sprintf(Lora_State.MasterName, "Unconnected");
    sprintf(Lora_State.RelayName, "NULL");
    Lora_State.RelaySAddr = 0;

    /*get page1_offset*/
    for (page1_offset = 0; page1_offset < 500; page1_offset++)
    {
        data = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8);
        if(data == Type_Lora_net)
        {
            Lora_Para_AT.SpreadingFactor = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) & 0x0F);
            Lora_Para_AT.BandWidth = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) & 0x03);
            Lora_Para_AT.NetOpen = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 3);
            Lora_Para_AT.Net_State = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 4);
            Lora_Para_AT.channel = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 5) & 0x7F;
            Lora_Para_AT.SAddrSelf = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 6) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 7));
            Lora_Para_AT.SAddrMaster = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 8) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 9));
            Lora_Para_AT.PanID = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 10) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 11));
            Lora_Para_AT.UART_BAUD = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 12) << 24) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 13) << 16)
             + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 14) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 15));
            page1_offset += ((Type_Lora_net_len + 7) / 8) - 1;
        }
        else if(data == Type_Lora_SelfName)
        {
            memset(Lora_State.SelfName, 0, Name_Size);
            memcpy(Lora_State.SelfName, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1), Name_Size - 1);
            Lora_State.SelfName[Name_Size - 1] = '\0';
            page1_offset += ((Type_Lora_SelfName_len + 7) / 8) - 1;
        }
        else if(data == Type_Lora_MasterName)
        {
            memset(Lora_State.MasterName, 0, Name_Size);
            memcpy(Lora_State.MasterName, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1), Name_Size - 1);
            Lora_State.MasterName[Name_Size - 1] = '\0';
            page1_offset += ((Type_Lora_MasterName_len + 7) / 8) - 1;
        }
        else if(data == Type_Lora_Relay)
        {
            memset(Lora_State.RelayName, 0, Name_Size);
            memcpy(Lora_State.RelayName, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1), Name_Size - 1);
            Lora_State.RelayName[Name_Size - 1] = '\0';
            Lora_State.RelaySAddr = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 17) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 18));
            page1_offset += ((Type_Lora_Relay_len + 7) / 8) - 1;
        }
        else
            break;
    }


    // 数值是否有效验证
    if (Lora_Para_AT.channel > 100)
        Lora_Para_AT.channel = 0;
    if (Lora_Para_AT.BandWidth > 2)
        Lora_Para_AT.BandWidth = 2;
    if (Lora_Para_AT.SpreadingFactor > 12 || Lora_Para_AT.SpreadingFactor < 7)
        Lora_Para_AT.SpreadingFactor = 7;
    if (Lora_Para_AT.Net_State > 2)
        Lora_Para_AT.Net_State = 0;
    if (Lora_Para_AT.PanID == 0xFFFF || Lora_Para_AT.PanID == BoardCast || Lora_Para_AT.PanID == 0x0000)
        Lora_Para_AT.PanID = Lora_State.chip_ID;
    if (Lora_Para_AT.SAddrSelf >= 0xFFFE || Lora_Para_AT.SAddrSelf == 0x0000)
        Lora_Para_AT.SAddrSelf = Lora_State.chip_ID;
    if (Lora_Para_AT.SAddrMaster >= 0xFFFE || Lora_Para_AT.SAddrMaster == 0x0000)
        Lora_Para_AT.SAddrMaster = 0;
    if (Lora_Para_AT.NetOpen == 2)
    { // 绑定主机
        Lora_Para_AT.SAddrMaster = 0;
        Lora_Para_AT.PanID = BoardCast;
        Lora_Para_AT.SAddrSelf = Lora_State.chip_ID;
    }
    if (Lora_Para_AT.UART_BAUD != 9600 && Lora_Para_AT.UART_BAUD != 19200 && Lora_Para_AT.UART_BAUD != 38400 && Lora_Para_AT.UART_BAUD != 57600 && Lora_Para_AT.UART_BAUD != 115200)
        Lora_Para_AT.UART_BAUD = 115200;
    uart_log_init(Lora_Para_AT.UART_BAUD);
    // 数据同步
    Lora_State.Channel = Lora_Para_AT.channel;
    Lora_State.SAddrMaster = Lora_Para_AT.SAddrMaster;
    Lora_State.SAddrSelf = Lora_Para_AT.SAddrSelf;
    Lora_State.PanID = Lora_Para_AT.PanID;
    Lora_State.Net_State = Lora_Para_AT.Net_State;
    Lora_State.NetOpen = Lora_Para_AT.NetOpen;
    memcpy(&Lora_Para_AT_Last, &Lora_Para_AT, sizeof(Lora_Para_AT));
    Lora_State.SelfName[Name_Size - 1] = '\0';
    Lora_State.MasterName[Name_Size - 1] = '\0';
    // 打印全部参数
    Debug_B("Lora_Para_AT.channel = %d\r\n", Lora_Para_AT.channel);
    Debug_B("Lora_Para_AT.BandWidth = %d\r\n", Lora_Para_AT.BandWidth);
    Debug_B("Lora_Para_AT.SpreadingFactor = %d\r\n", Lora_Para_AT.SpreadingFactor);
    Debug_B("Lora_Para_AT.SAddrMaster = %04X\r\n", Lora_Para_AT.SAddrMaster);
    Debug_B("Lora_Para_AT.SAddrSelf = %04X\r\n", Lora_Para_AT.SAddrSelf);
    Debug_B("Lora_Para_AT.PanID = %04X\r\n", Lora_Para_AT.PanID);
    Debug_B("Lora_Para_AT.Net_State = %d\r\n", Lora_Para_AT.Net_State);
    Debug_B("Lora_Para_AT.NetOpen = %d\r\n", Lora_Para_AT.NetOpen);
    Debug_B("Lora_State.SelfName = %s\r\n", Lora_State.SelfName);
    Debug_B("Lora_State.MasterName = %s\r\n", Lora_State.MasterName);
    Debug_B("Lora_State.RelayName = %s\r\n", Lora_State.RelayName);
}

// page 2 存储类型
#define Type_Slaver_Para        0x01
#define Type_Slaver_Para_len    32
/**
 * Lora_AsData_Add
 * @brief 主机有效-将AS数组内的数据保存在Flash中
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Add(uint8_t ID)
{
    // 8字节Mac + 2字节Saddr + 1字节ID + 1字节状态 + 4字节时间 + 16字节Name
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (ID >= Device_Num_Max)
        return;
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < Device_Num_Max; i++)
        {
            if (Associated_devices[i].SAddr)
            {
                Lora_AsData_Add(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Slaver_Para;
    temp_data[1] = ID;
    memcpy(&temp_data[2], Associated_devices[ID].Mac, 8);
    temp_data[10] = Associated_devices[ID].SAddr >> 8;
    temp_data[11] = Associated_devices[ID].SAddr;
    memcpy(&temp_data[16], Associated_devices[ID].Name, 16);
    temp_data[31] = '\0';
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, ((Type_Slaver_Para + 7) / 8) * 8);
    page2_offset += (Type_Slaver_Para + 7) / 8;
}

/**
 * Lora_AsData_Del
 * @brief 主机有效-将某个ID数据删除
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Del(uint8_t ID)
{
    /* 替代方案:写一个新的数据,其内部数据均为0 */
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < Device_Num_Max; i++)
        {
            if (Associated_devices[i].Net_State == Net_JoinGateWay)
            {
                Lora_AsData_Add(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Slaver_Para;
    temp_data[1] = ID;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, ((Type_Slaver_Para + 7) / 8) * 8);
    page2_offset += (Type_Slaver_Para + 7) / 8;
}

/**
 * Lora_AsData_Syn
 * @brief 主机有效-从flash内读取AS数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_AsData_Syn()
{
    uint8_t ID;
    // 主机连接的设备参数同步 每32个字节一组数据，根据第12个字节是否为0判断数据是否有效
    for (page2_offset = 0; page2_offset < 500; page2_offset++)
    {
        if(*(uint8_t *)(FlashData2_ADDR + page2_offset * 8) == Type_Slaver_Para 
        && *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1) < Device_Num_Max)
        {
            ID = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1);
            Associated_devices[ID].Mac[0] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 2);
            Associated_devices[ID].Mac[1] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 3);
            Associated_devices[ID].Mac[2] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 4);
            Associated_devices[ID].Mac[3] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 5);
            Associated_devices[ID].Mac[4] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 6);
            Associated_devices[ID].Mac[5] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 7);
            Associated_devices[ID].Mac[6] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 8);
            Associated_devices[ID].Mac[7] = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 9);
            Associated_devices[ID].SAddr = (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 10) << 8)
             + (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 11));
            memcpy(Associated_devices[ID].Name, (uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 16), 16);
            Associated_devices[ID].Name[16] = '\0';
            page2_offset += ((Type_Slaver_Para_len + 7) / 8) - 1;
        }
        else
            break;
    }
    for (uint8_t i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr)
        {
            Debug_B("ID:%d SAddr:%04x Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, Associated_devices[i].SAddr,
                    Associated_devices[i].Mac[0], Associated_devices[i].Mac[1], Associated_devices[i].Mac[2], Associated_devices[i].Mac[3],
                    Associated_devices[i].Mac[4], Associated_devices[i].Mac[5], Associated_devices[i].Mac[6], Associated_devices[i].Mac[7]);
        }
    }
}