

#include "Lora_core.h"

#define TAG "lora_flash"
uint16_t page1_offset, page2_offset, page3_offset;

void LoRa_NetPara_Save(uint8_t type)
{
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (page1_offset >= 500)
    {
        page1_offset = 0;
        LOG_I(TAG, "NetPara erase\r\n");
        flash_erase_page(FlashData1_ADDR);
        LoRa_NetPara_Save(Type_Lora_net);
        LoRa_NetPara_Save(Type_Lora_SelfName);
        LoRa_NetPara_Save(Type_Lora_MasterName);
    }
    switch (type)
    {
    case Type_Lora_net:
        /* code */
        temp_data[0] = type;
        temp_data[1] = LoRaBackup.SpreadingFactor & 0x0F;
        temp_data[2] = LoRaBackup.BandWidth & 0x03;
        temp_data[3] = LoRaDevice.NetMode;
        temp_data[4] = LoRaDevice.Net_State;
        temp_data[5] = LoRaBackup.channel & 0x7F;
        temp_data[6] = LoRaBackup.SAddr >> 8;
        temp_data[7] = LoRaBackup.SAddr;
        temp_data[8] = LoRaDevice.Master.shortAddress >> 8;
        temp_data[9] = LoRaDevice.Master.shortAddress;
        temp_data[10] = LoRaBackup.PanID >> 8;
        temp_data[11] = LoRaBackup.PanID;
        temp_data[12] = LoRaBackup.UART_Baud >> 24;
        temp_data[13] = LoRaBackup.UART_Baud >> 16;
        temp_data[14] = LoRaBackup.UART_Baud >> 8;
        temp_data[15] = LoRaBackup.UART_Baud;
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Type_Lora_net_len);
        page1_offset += (Type_Lora_net_len / 8);
        break;
    case Type_Lora_SelfName:
        temp_data[0] = type;
        memcpy(&temp_data[1], LoRaDevice.Self.name, Name_Size - 1);
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Type_Lora_SelfName_len);
        page1_offset += (Type_Lora_SelfName_len / 8);
        break;
    case Type_Lora_MasterName:
        temp_data[0] = type;
        memcpy(&temp_data[1], LoRaDevice.Master.name, Name_Size - 1);
        memcpy(&temp_data[Name_Size], LoRaDevice.Master.Mac, 8);
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, Type_Lora_MasterName_len);
        page1_offset += (Type_Lora_MasterName_len / 8);
        break;
    default:
        break;
    }
    LOG_I(TAG, "page1 data save, release %d\r\n", 500 - page1_offset);
}

void LoRaNetParaSyn()
{
    uint32_t data;
    LoRaDevice.NetMode = SearchForMaster; // 默认为搜寻主机
    for (page1_offset = 0; page1_offset < 500; page1_offset++)
    { // flash 同步
        data = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8);
        if (data == Type_Lora_net)
        {
            LoRaBackup.SpreadingFactor = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) & 0x0F);
            LoRaBackup.BandWidth = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) & 0x03);
            LoRaDevice.NetMode = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 3);
            LoRaDevice.Net_State = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 4);
            LoRaBackup.channel = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 5) & 0x7F;
            LoRaBackup.SAddr = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 6) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 7));
            LoRaDevice.Master.shortAddress = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 8) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 9));
            LoRaBackup.PanID = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 10) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 11));
            LoRaBackup.UART_Baud = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 12) << 24) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 13) << 16) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 14) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 15));
            page1_offset += ((Type_Lora_net_len / 8) - 1);
        }
        else if (data == Type_Lora_SelfName)
        {
            memset(LoRaDevice.Self.name, 0, Name_Size);
            memcpy(LoRaDevice.Self.name, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1), Name_Size - 1);
            page1_offset += ((Type_Lora_SelfName_len / 8) - 1);
        }
        else if (data == Type_Lora_MasterName)
        {
            memset(LoRaDevice.Master.name, 0, Name_Size);
            memcpy(LoRaDevice.Master.name, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1), Name_Size - 1);
            memcpy(LoRaDevice.Master.Mac, (uint8_t *)(FlashData1_ADDR + page1_offset * 8 + Name_Size), 8);
            page1_offset += ((Type_Lora_MasterName_len / 8) - 1);
        }
        else
            break;
    }
    if (LoRaBackup.SpreadingFactor < 7 || LoRaBackup.SpreadingFactor > 12)
        LoRaBackup.SpreadingFactor = 7;
    if (LoRaBackup.BandWidth > 2)
        LoRaBackup.BandWidth = 2;
    if (LoRaBackup.channel > 100)
        LoRaBackup.channel = 0;
    if (LoRaDevice.Net_State > 1)
        LoRaDevice.Net_State = 0;
    if (LoRaDevice.NetMode > 2)
        LoRaDevice.NetMode = 0;
    if (LoRaBackup.SAddr >= 0xFFFE || LoRaBackup.SAddr == 0x0000)
        LoRaBackup.SAddr = LoRaDevice.chip_ID;
    if (LoRaBackup.PanID == 0xFFFF || LoRaBackup.PanID == 0x0000)
        LoRaBackup.PanID = LoRaDevice.chip_ID;
    if (LoRaBackup.UART_Baud != 9600 && LoRaBackup.UART_Baud != 19200 && LoRaBackup.UART_Baud != 38400 && LoRaBackup.UART_Baud != 57600 && LoRaBackup.UART_Baud != 115200)
	{
		LoRaBackup.UART_Baud = 9600;
	}	
	uart_log_init(LoRaBackup.UART_Baud);
    if (LoRaDevice.Self.name[0] == 0 || LoRaDevice.Self.name[0] == 0xFF)
    {
        sprintf(LoRaDevice.Self.name, "ID:%04x", LoRaDevice.chip_ID);
        LOG_W(TAG, "Self name not found\r\n");
    }
    if (LoRaDevice.Master.name[0] == 0 || LoRaDevice.Master.name[0] == 0xFF || LoRaDevice.Master.shortAddress == 0 || LoRaDevice.Master.shortAddress >= BoardCast)
    {
        memset(&LoRaDevice.Master, 0, sizeof(LoRa_Node_t));
        sprintf((char *)LoRaDevice.Master.name, "Unconnected");
        LOG_W(TAG, "Master not found\r\n");
    }
    // 数据同步
    LoRaDevice.channel = LoRaBackup.channel;
    LoRaDevice.Self.shortAddress = LoRaBackup.SAddr;
    LoRaDevice.PanID = LoRaBackup.PanID;
    LoRaDevice.BandWidth = LoRaBackup.BandWidth;
    LoRaDevice.SpreadingFactor = LoRaBackup.SpreadingFactor;
    if (LoRaDevice.NetMode == SearchForMaster)
        LoRaDevice.PanID = BoardCast; // 只修改Device,不修改backup
    LOG_I(TAG, "NetState:%d, NetMode:%d\r\n", LoRaDevice.Net_State, LoRaDevice.NetMode);
    LOG_I(TAG, "channel:%d, PanID:%04x SelfAddr:%04x, MasterAddr:%04x \r\n", LoRaDevice.channel, LoRaDevice.PanID, LoRaDevice.Self.shortAddress, LoRaDevice.Master.shortAddress);
    LOG_I(TAG, "SpreadingFactor:%d, BandWidth:%d \r\n", LoRaDevice.SpreadingFactor, LoRaDevice.BandWidth);
    LOG_I(TAG, "LoRaDevice.Master.name = %s\r\n", LoRaDevice.Master.name);
    LOG_I(TAG, "LoRaDevice.Self.name = %s\r\n", LoRaDevice.Self.name);
    LOG_I(TAG, "LoRaDevice.UART_Baud = %d\r\n", (int)LoRaBackup.UART_Baud);
}
/**
 * Lora_AsData_Add
 * @brief 主机有效-将AS数组内的数据保存在Flash中
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaAddSlaver(uint8_t ID)
{
    // 8字节Mac + 2字节Saddr + 1字节ID + 1字节状态 + 4字节时间 + 16字节Name
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (ID >= MAX_CHILDREN)
        return;
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        LOG_I(TAG, "Slaver erase\r\n");
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < MAX_CHILDREN; i++)
        {
            if (LoRaDevice.Slaver[i].shortAddress)
            {
                LoRaAddSlaver(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Lora_Slaver;
    temp_data[1] = ID;
    memcpy(&temp_data[2], LoRaDevice.Slaver[ID].Mac, 8);
    temp_data[10] = LoRaDevice.Slaver[ID].shortAddress >> 8;
    temp_data[11] = LoRaDevice.Slaver[ID].shortAddress;
    memcpy(&temp_data[16], LoRaDevice.Slaver[ID].name, 16);
    temp_data[31] = '\0';
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, Type_Lora_Slaver_len);
    page2_offset += (Type_Lora_Slaver_len / 8);
    LOG_I(TAG, "AddSlaver:%d, SAddr:%d, name:%s\r\n", ID, LoRaDevice.Slaver[ID].shortAddress, LoRaDevice.Slaver[ID].name);
    LOG_I(TAG, "page release %d\r\n", 500 - page2_offset);
}

/**
 * LoRaDelSlaver
 * @brief 主机有效-将某个ID数据删除
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaDelSlaver(uint8_t ID)
{
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (ID >= MAX_CHILDREN)
        return;
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        LOG_I(TAG, "Slaver erase\r\n");
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < MAX_CHILDREN; i++)
        {
            if (LoRaDevice.Slaver[i].shortAddress)
            {
                LoRaAddSlaver(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Lora_Slaver;
    temp_data[1] = ID;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, Type_Lora_Slaver_len);
    page2_offset += (Type_Lora_Slaver_len / 8);
    LOG_I(TAG, "DelSlaver:%d, SAddr:%d \r\n", ID, LoRaDevice.Slaver[ID].shortAddress);
    LOG_I(TAG, "page release %d\r\n", 500 - page2_offset);
}

/**
 * Lora_AsData_Syn
 * @brief 主机有效-从flash内读取AS数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaSlaverSyn()
{
    uint8_t ID;
    // 主机连接的设备参数同步 每32个字节一组数据，根据第12个字节是否为0判断数据是否有效
    for (page2_offset = 0; page2_offset < 500; page2_offset++)
    {
        if (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8) == Type_Lora_Slaver && *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1) < MAX_CHILDREN)
        {
            ID = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1);
            memcpy(LoRaDevice.Slaver[ID].Mac, (uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 2), 8);
            LoRaDevice.Slaver[ID].shortAddress = (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 10) << 8) + (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 11));
            memcpy(LoRaDevice.Slaver[ID].name, (uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 16), 16);
            LoRaDevice.Slaver[ID].name[Name_Size - 1] = '\0';
            page2_offset += (Type_Lora_Slaver_len / 8) - 1;
        }
        else
            break;
    }
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress)
        {
            LOG_I(TAG, "slaver ID:%d SAddr:%04x Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X name:%s\r\n", i, LoRaDevice.Slaver[i].shortAddress,
                        LoRaDevice.Slaver[i].Mac[0], LoRaDevice.Slaver[i].Mac[1], LoRaDevice.Slaver[i].Mac[2], LoRaDevice.Slaver[i].Mac[3],
                        LoRaDevice.Slaver[i].Mac[4], LoRaDevice.Slaver[i].Mac[5], LoRaDevice.Slaver[i].Mac[6], LoRaDevice.Slaver[i].Mac[7],
                        LoRaDevice.Slaver[i].name);
        }
    }
}

void LoRaFlashdataSyn()
{
    LoRaNetParaSyn();
    LoRaSlaverSyn();
}