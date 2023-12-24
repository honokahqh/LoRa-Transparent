#include "lora_core.h"
/**
 * Cmd_BeaconRequest
 * @brief Uses the local chipID as an offset and the subdevice's chipID as a short address to send to the subdevice.
 *        Registers in the connected device array and sets the network stage to Net_Joining.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_BeaconRequest()
{
    uint8_t Temp_Data[20], i;
    uint16_t short_Addr;
    char macStr[24], name[16], infoStr[60];
    memset(Temp_Data, 0, 20);
    memset(macStr, 0, 24);
    memset(name, 0, 16);
    memset(infoStr, 0, 60);
    if (Lora_State.NetOpen != Net_SearchSlave)
        return;
    /* 注册一半失败or其他原因导致的重新注册,原数据删除 */
    i = Compare_MAC(&Lora_State.Rx_Data[Data_Addr]);
    if (i != 0xFF)
	{
		memset(&Associated_devices[i], 0, sizeof(associated_devices_t));
		goto RegisterStart;
	}
        

    if (CompareMac(&Lora_State.Rx_Data[Data_Addr], Register_Device.Mac) == 0)
    { // 通知检索到新设备
        MacToString(&Lora_State.Rx_Data[Data_Addr], macStr);
        strncpy(name, (char *)&Lora_State.Rx_Data[Data_Addr + 8], Lora_State.Rx_Data[Len_Addr] - 8);
        // 将设备的信息格式化为字符串
        sprintf(infoStr, "REG__%s__%s__%d", name, macStr, Lora_State.Rx_RSSI);
        BLE_SendData((uint8_t *)infoStr, strlen(infoStr));
        return;
    }
	RegisterStart:
    i = Get_IDLE_ID();
    if (i == 0xFF)
        return;
    memset(&Associated_devices[i], 0, sizeof(associated_devices_t));
    Associated_devices[i].Net_State = Net_Joining;
    Associated_devices[i].DeviceType = Lora_State.Rx_DevType;
    Associated_devices[i].chip_ID = Lora_State.Rx_SAddr;
    Associated_devices[i].RSSI = Lora_State.Rx_RSSI;
    Associated_devices[i].Timeout = Register_Timeout;
    memcpy(Associated_devices[i].Mac, &Lora_State.Rx_Data[Data_Addr], 8);
    strncpy(Associated_devices[i].Name, (char *)&Lora_State.Rx_Data[Data_Addr + 8], Lora_State.Rx_Data[Len_Addr] - 8);
    short_Addr = Lora_State.Rx_SAddr + Lora_State.PanID;
    while (Compare_ShortAddr(short_Addr) != 0xFF || short_Addr == 0 || short_Addr > 0xFFFD)
    { // 短地址非0且该地址未被注册
        short_Addr++;
    }
    Associated_devices[i].SAddr = short_Addr;
    Temp_Data[0] = Associated_devices[i].SAddr >> 8;
    Temp_Data[1] = Associated_devices[i].SAddr & 0xFF;
    Temp_Data[2] = Lora_State.Channel;
    Temp_Data[3] = Lora_Para_AT.BandWidth;
    Temp_Data[4] = Lora_Para_AT.SpreadingFactor;
    Temp_Data[5] = 1;
    Debug_B("Send Beacon:%04x\r\n", Associated_devices[i].SAddr);
    CusProfile_Send(Associated_devices[i].chip_ID, Beacon, Temp_Data, 6, TRUE);
}

/**
 * Cmd_SlaverInNet
 * @brief Handles the notification of a sub-device joining the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_SlaverInNet()
{
    uint8_t i;
    char macStr[24], infoStr[60];
    if (Lora_State.NetOpen != Net_SearchSlave)
        return;
    i = Compare_ShortAddr(Lora_State.Rx_SAddr);
    if (i != 0xFF && Associated_devices[i].Net_State == Net_JoinGateWay)
    { // 已经注册过了,可能是对方未收到上条device annonce
        CusProfile_Send(Lora_State.Rx_SAddr, DeviceAnnonce, NULL, 0, TRUE);
        return;
    }

    Associated_devices[i].RSSI = Lora_State.Rx_RSSI;
    Associated_devices[i].Wait_ACK = 0;
    Associated_devices[i].Net_State = Net_JoinGateWay;
    Lora_AsData_Add(i);
    Debug_B("Send DeviceAnnonce\r\n");
    Debug_B("ID:%d SAddr:%04X Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, Associated_devices[i].SAddr,
            Associated_devices[i].Mac[0], Associated_devices[i].Mac[1], Associated_devices[i].Mac[2], Associated_devices[i].Mac[3],
            Associated_devices[i].Mac[4], Associated_devices[i].Mac[5], Associated_devices[i].Mac[6], Associated_devices[i].Mac[7]);
    memset(infoStr, 0, 60);
    memcpy(infoStr, Lora_State.Mac, 8);
    sprintf(&infoStr[8], "%s", Lora_State.SelfName);
    CusProfile_Send(Lora_State.Rx_SAddr, DeviceAnnonce, (uint8_t*)infoStr, strnlen(Lora_State.SelfName, 15) + 8, TRUE);

    // 将设备的信息格式化为字符串
    memset(macStr, 0, 24);
    memset(infoStr, 0, 60);
    MacToString(Associated_devices[i].Mac, macStr);
    sprintf(infoStr, "CON__%s__%s__%d", Associated_devices[i].Name, macStr, Associated_devices[i].RSSI);
    BLE_SendData((uint8_t *)infoStr, strlen(infoStr));
}

/**
 * Cmd_Slaver_Request_Leave
 * @brief Handles a sub-device's request to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Slaver_Request_Leave()
{
    uint8_t i;
    i = Compare_ShortAddr(Lora_State.Rx_SAddr);
    Debug_B("Send Master_Leave_ACK\r\n");
    CusProfile_Send(Associated_devices[i].SAddr, Master_Leave_ACK, NULL, 0, TRUE);
    memset(&Associated_devices[i], 0, sizeof(associated_devices_t));
    Lora_AsData_Del(i);
}

/**
 * Cmd_HeartBeat
 * @brief Reserved for a future function related to maintaining network connection (typically used for sending periodic signals to ensure active connections).
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_HeartBeat()
{
}

/**
 * PCmd_Master_Request_Leave
 * @brief Function used by the master device to request a sub-device to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Master_Request_Leave(uint8_t ID)
{
    Debug_B("Send Master_Request_Leave\r\n");
    CusProfile_Send(Associated_devices[ID].SAddr, Master_Request_Leave, NULL, 0, FALSE);
    memset(&Associated_devices[ID], 0, sizeof(associated_devices_t)); // 将数据全部清除，之后接收到该短地址的数据会直接要求离网
    Lora_AsData_Del(ID);
}

/**
 * PCmd_Master_Request_Leave
 * @brief Function used by the master device to request a sub-device to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Lora_SendData()
{
    if(Lora_State.SAddrMaster == Lora_State.Rx_SAddr)
        Lora_State.RssiMaster = Lora_State.Rx_RSSI;
    for(uint8_t i = 0;i < Device_Num_Max;i++)
    {
        if(Associated_devices[i].SAddr == Lora_State.Rx_SAddr)
        {
            Associated_devices[i].RSSI = Lora_State.Rx_RSSI;
            break;
        }   
    }
//    uint8_t rssi[2];
//    rssi[0] = Lora_State.Rx_RSSI >> 8;
//    rssi[1] = Lora_State.Rx_RSSI & 0xFF;
    UART_SendData(&Lora_State.Rx_Data[Data_Addr], Lora_State.Rx_Data[Len_Addr]);
//    UART_SendData(rssi, 2);
}   