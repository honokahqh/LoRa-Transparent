#include "lora_core.h"

#define TAG "LoRa externalCmd"

const static uint8_t BindFail[] = "BindFail\r\n";
const static uint8_t BindSuccess[] = "BindSuccess\r\n";
// 设置从机名称
void PCmd_LoRa_SetSlaverName(char *name, uint8_t ID)
{
    if (ID >= MAX_CHILDREN)
        return;
    memset(LoRaDevice.Slaver[ID].name, 0, Name_Size);
    strncpy(LoRaDevice.Slaver[ID].name, name, Name_Size - 1);
    CusProfile_Send(LoRaDevice.Slaver[ID].shortAddress, LoRa_SetSlaverName, (uint8_t *)name,
                    strnlen(name, Name_Size - 1), FALSE);
    LoRaAddSlaver(ID);
}

// 从机对设置名称命令处理
void Cmd_LoRa_SetSlaverName()
{
    char BleBuffer[50];
    memset(BleBuffer, 0, sizeof(BleBuffer));
    memset(LoRaDevice.Self.name, 0, Name_Size);
    strncpy(LoRaDevice.Self.name, (char *)&LoRaPacket.Rx_Data[Data_Addr], LoRaPacket.Rx_Data[Len_Addr]);
    LoRaDevice.Self.name[Name_Size - 1] = '\0';
    sprintf(BleBuffer, "AT+NAME=乐清%s\r\n", LoRaDevice.Self.name);
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    delay_ms(500);
    sprintf(BleBuffer, "AT+REBOOT=1\r\n");
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    LoRa_NetPara_Save(Type_Lora_SelfName);
}

// 设置自身名称
void PCmd_LoRa_SetSelfName(char *name)
{
    memset(LoRaDevice.Self.name, 0, Name_Size);
    strncpy(LoRaDevice.Self.name, name, Name_Size - 1);
    CusProfile_Send(BoardCast, LoRa_SetSelfName, (uint8_t *)LoRaDevice.Self.name,
                    strnlen(LoRaDevice.Self.name, Name_Size - 1), FALSE);
    LoRa_NetPara_Save(Type_Lora_SelfName);
}

// 对设备设置自身名称处理
void Cmd_LoRa_SetSelfName()
{
    if (LoRaPacket.Rx_SAddr == LoRaDevice.Master.shortAddress)
    {
        memset(LoRaDevice.Master.name, 0, Name_Size);
        strncpy(LoRaDevice.Master.name, (char *)&LoRaPacket.Rx_Data[Data_Addr], LoRaPacket.Rx_Data[Len_Addr]);
        LoRaDevice.Master.name[Name_Size - 1] = '\0';
        LoRa_NetPara_Save(Type_Lora_MasterName);
    }
    else
    {
        for (uint8_t i = 0; i < MAX_CHILDREN; i++)
        {
            if (LoRaDevice.Slaver[i].shortAddress == LoRaPacket.Rx_SAddr)
            {
                memset(LoRaDevice.Slaver[i].name, 0, sizeof(LoRaDevice.Slaver[i].name));
                strncpy(LoRaDevice.Slaver[i].name, (char *)&LoRaPacket.Rx_Data[Data_Addr], LoRaPacket.Rx_Data[Len_Addr]);
                LoRaDevice.Slaver[i].name[Name_Size - 1] = '\0';
                LoRaAddSlaver(i);
                break;
            }
        }
    }
}

static uint16_t BindSAddrSrc, BindSAddrDst, BestRouter;
static int16_t RouterRssi;
// 源节点发起请求
void PCmd_LoRa_Bind_Init(uint16_t shortAddress)
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.RelayGroup[i].A.shortAddress == 0x0000)
            break;
    }
    if (i == MAX_CHILDREN)
    {
        LOG_I(TAG, "%s, 中继节点已满\r\n", __func__);
        BLE_SendData((uint8_t *)BindFail, sizeof(BindFail) - 1);
        return;
    }

    uint8_t shortAddress_buffer[2];
    BestRouter = 0;
    RouterRssi = -200;
    shortAddress_buffer[0] = shortAddress >> 8;
    shortAddress_buffer[1] = shortAddress & 0xFF;
    BindSAddrSrc = LoRaDevice.Self.shortAddress;
    BindSAddrDst = shortAddress;
    LoRaPacket.Wait_ACK = LoRa_Bind_Init;
    if (LoRaDevice.SpreadingFactor - LoRaDevice.BandWidth == 5)
        LoRaPacket.AckTimeout = 2;
    else if (LoRaDevice.SpreadingFactor - LoRaDevice.BandWidth == 6)
        LoRaPacket.AckTimeout = 3;
    else if (LoRaDevice.SpreadingFactor - LoRaDevice.BandWidth == 7)
        LoRaPacket.AckTimeout = 5;
    else
        LoRaPacket.AckTimeout = 5;
    CusProfile_Send(BoardCast, LoRa_Bind_Init, shortAddress_buffer, 2, FALSE);
}

// 中继回复源节点-列表内有目标节点
void Cmd_LoRa_Bind_Init()
{
    uint8_t i = 0;
    if (LoRaPacket.Rx_RSSI < Neighbor_MinRSSI)
        return;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.RelayGroup[i].A.shortAddress == 0x0000)
            break;
    }
    if (i == MAX_CHILDREN)
    {
        LOG_I(TAG, "%s, 中继节点已满\r\n", __func__);
        return;
    }
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Neighbor[i].shortAddress == ((LoRaPacket.Rx_Data[Data_Addr] << 8) + LoRaPacket.Rx_Data[Data_Addr + 1]))
        {
            uint8_t temp_data[4];
            temp_data[0] = LoRaDevice.Neighbor[i].RSSI >> 8;
            temp_data[1] = LoRaDevice.Neighbor[i].RSSI & 0xFF;
            LOG_I(TAG, "查询到对应邻居节点\n");
            switch (LoRaDevice.SpreadingFactor - LoRaDevice.BandWidth)
            {
            case 5:
                delay_ms(50 * (LoRaDevice.chip_ID % 20));
                break;
            case 6:
                delay_ms(100 * (LoRaDevice.chip_ID % 20));
                break;
            case 7:
                delay_ms(200 * (LoRaDevice.chip_ID % 20));
                break;
            default:
                delay_ms(200 * (LoRaDevice.chip_ID % 20));
                break;
            }
            BindSAddrSrc = LoRaPacket.Rx_SAddr;
            BindSAddrDst = (LoRaPacket.Rx_Data[Data_Addr] << 8) + LoRaPacket.Rx_Data[Data_Addr + 1];
            CusProfile_Send(LoRaPacket.Rx_SAddr, LoRa_Bind_NbrReply, temp_data, 2, TRUE);
            return;
        }
    }
}

// 源节点记录最优中继
void Cmd_LoRa_Bind_NbrReply()
{
    int16_t DstRssi = (LoRaPacket.Rx_Data[Data_Addr] << 8) + LoRaPacket.Rx_Data[Data_Addr + 1];
    if (RouterRssi < LoRaPacket.Rx_RSSI + DstRssi) // 信号强度和最大
    {
        RouterRssi = LoRaPacket.Rx_RSSI + DstRssi;
        BestRouter = LoRaPacket.Rx_SAddr;
    }
    // CusProfile_Send(LoRaPacket.Rx_SAddr, LoRa_Bind_SrcAck, NULL, 0, TRUE); 等待cmd超时
}

// 源节点发起绑定
void PCmd_LoRa_Bind_RelayTry()
{
    uint8_t temp_data[18];
    memset(temp_data, 0, sizeof(temp_data));
    memcpy(&temp_data[2], LoRaDevice.Self.name, Name_Size - 1);
    temp_data[0] = LoRaDevice.Self.shortAddress >> 8;
    temp_data[1] = LoRaDevice.Self.shortAddress & 0xFF;
    LoRaPacket.Wait_ACK = 0;
    if (BestRouter != 0x0000)
        CusProfile_Send(LoRaPacket.Rx_SAddr, LoRa_Bind_SrcAck, (uint8_t*)LoRaDevice.Self.name, 18, TRUE);
}

// 中继节点向目标节点发起绑定
void Cmd_LoRa_Bind_SrcAck()
{
    uint8_t temp_data[36];
    memcpy(temp_data, &LoRaPacket.Rx_Data[Data_Addr], 18);
    temp_data[18] = LoRaDevice.Self.shortAddress >> 8;
    temp_data[19] = LoRaDevice.Self.shortAddress & 0xFF;
    memcpy(&temp_data[20], LoRaDevice.Self.name, Name_Size - 1);
    CusProfile_Send(BindSAddrDst, LoRa_Bind_RelayTry, temp_data, 36, TRUE);
}

// 目标节点回复中继节点
void Cmd_LoRa_Bind_RelayTry()
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.RelayGroup[i].A.shortAddress == 0x0000)
            break;
    }
    if (i == MAX_CHILDREN)
    {
        LOG_I(TAG, "%s, 中继节点已满\r\n", __func__);
        return;
    }
    uint8_t temp_data[54];
    // 此时 目标节点记录本链路
    memcpy(temp_data, &LoRaPacket.Rx_Data[Data_Addr], 36);
    LoRaDevice.RelayGroup[i].A.shortAddress = (temp_data[0] << 8) + temp_data[1];
    memcpy(LoRaDevice.RelayGroup[i].A.name, &temp_data[2], Name_Size);
    LoRaDevice.RelayGroup[i].Relay.shortAddress = (temp_data[18] << 8) + temp_data[19];
    memcpy(LoRaDevice.RelayGroup[i].Relay.name, &temp_data[20], Name_Size);
    LoRaDevice.RelayGroup[i].B.shortAddress = LoRaDevice.Self.shortAddress;
    memcpy(LoRaDevice.RelayGroup[i].B.name, LoRaDevice.Self.name, Name_Size);

    temp_data[36] = LoRaDevice.Self.shortAddress >> 8;
    temp_data[37] = LoRaDevice.Self.shortAddress & 0xFF;
    memcpy(&temp_data[38], LoRaDevice.Self.name, Name_Size - 1);
    CusProfile_Send(LoRaPacket.Rx_SAddr, LoRa_Bind_TgtAck, temp_data, 54, TRUE);
}

// 中继节点回复源节点
void Cmd_LoRa_Bind_TgtAck()
{
    uint8_t temp_data[54], i;
    // 此时 中继节点记录本链路
    memcpy(temp_data, &LoRaPacket.Rx_Data[Data_Addr], 54);
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.RelayGroup[i].A.shortAddress == 0x0000)
            break;
    }
    if (i == MAX_CHILDREN)
    {
        LOG_I(TAG, "%s, 中继节点已满\r\n", __func__);
        return;
    }
    LoRaDevice.RelayGroup[i].A.shortAddress = (temp_data[0] << 8) + temp_data[1];
    memcpy(LoRaDevice.RelayGroup[i].A.name, &temp_data[2], Name_Size);
    LoRaDevice.RelayGroup[i].Relay.shortAddress = (temp_data[18] << 8) + temp_data[19];
    memcpy(LoRaDevice.RelayGroup[i].Relay.name, &temp_data[20], Name_Size);
    LoRaDevice.RelayGroup[i].B.shortAddress = (temp_data[36] << 8) + temp_data[37];
    memcpy(LoRaDevice.RelayGroup[i].B.name, &temp_data[38], Name_Size);
    CusProfile_Send(BindSAddrSrc, LoRa_Bind_Success, temp_data, 54, TRUE);
}

// 源节点回复中继节点
void Cmd_LoRa_Bind_Success()
{
    // 此时 源节点记录本链路
    uint8_t temp_data[54];
    memcpy(temp_data, &LoRaPacket.Rx_Data[Data_Addr], 54);
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.RelayGroup[i].A.shortAddress == 0x0000)
            break;
    }
    if (i == MAX_CHILDREN)
    {
        LOG_I(TAG, "%s, 中继节点已满\r\n", __func__);
        return;
    }
    LoRaDevice.RelayGroup[i].A.shortAddress = (temp_data[0] << 8) + temp_data[1];
    memcpy(LoRaDevice.RelayGroup[i].A.name, &temp_data[2], Name_Size);
    LoRaDevice.RelayGroup[i].Relay.shortAddress = (temp_data[18] << 8) + temp_data[19];
    memcpy(LoRaDevice.RelayGroup[i].Relay.name, &temp_data[20], Name_Size);
    LoRaDevice.RelayGroup[i].B.shortAddress = (temp_data[36] << 8) + temp_data[37];
    memcpy(LoRaDevice.RelayGroup[i].B.name, &temp_data[38], Name_Size);
    CusProfile_Send(LoRaPacket.Rx_SAddr, LoRa_Bind_Success_ACK, NULL, 0, TRUE);
}