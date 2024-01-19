#include "lora_core.h"

#define TAG "LoRa cmd_m"
/**
 * Cmd_BeaconRequest
 * @brief Uses the local chipID as an offset and the subdevice's chipID as a short address to send to the subdevice.
 *        Registers in the connected device array and sets the network stage to Net_Joining.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_BeaconRequest()
{
    uint8_t Temp_Data[6], i;
    uint16_t short_Addr;
    char macStr[24], name[16], infoStr[60];
    memset(Temp_Data, 0, 6);
    memset(macStr, 0, 24);
    memset(name, 0, 16);
    memset(infoStr, 0, 60);
    if (LoRaDevice.NetMode != SearchForSlave)
        return;
    i = Compare_MAC(&LoRaPacket.Rx_Data[Data_Addr]);
    if (i != 0xFF)
    { // 设备重复注册,直接许可
        memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
        goto RegisterStart;
    }

    if (CompareMac(&LoRaPacket.Rx_Data[Data_Addr], RegisterDevice.device.Mac) == 0)
    { // 与Register node mac不同,通知检索到新设备后返回
        MacToString(&LoRaPacket.Rx_Data[Data_Addr], macStr);
        strncpy(name, (char *)&LoRaPacket.Rx_Data[Data_Addr + 8], LoRaPacket.Rx_Data[Len_Addr] - 8);
        // 将设备的信息格式化为字符串
        sprintf(infoStr, "REG__%s__%s__%d", name, macStr, LoRaPacket.Rx_RSSI);
        BLE_SendData((uint8_t *)infoStr, strlen(infoStr));
        return;
    }
RegisterStart:
    i = Get_IDLE_ID();
    if (i == 0xFF)
        return;
    memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
    LoRaDevice.Slaver[i].DevcieType = LoRaPacket.Rx_DevType;
    LoRaDevice.Slaver[i].RSSI = LoRaPacket.Rx_RSSI;
    LoRaDevice.Slaver[i].timeout = Register_Timeout;
    memcpy(LoRaDevice.Slaver[i].Mac, &LoRaPacket.Rx_Data[Data_Addr], 8);
    strncpy(LoRaDevice.Slaver[i].name, (char *)&LoRaPacket.Rx_Data[Data_Addr + 8], LoRaPacket.Rx_Data[Len_Addr] - 8);
    short_Addr = LoRaPacket.Rx_SAddr + LoRaDevice.PanID;
    while (Compare_ShortAddr(short_Addr) != 0xFF || short_Addr == 0 || short_Addr > 0xFFFD)
    { // 短地址非0且该地址未被注册
        short_Addr++;
    }
    LoRaDevice.Slaver[i].shortAddress = short_Addr;

    Temp_Data[0] = LoRaDevice.Slaver[i].shortAddress >> 8;
    Temp_Data[1] = LoRaDevice.Slaver[i].shortAddress & 0xFF;
    Temp_Data[2] = LoRaDevice.channel;
    Temp_Data[3] = LoRaDevice.BandWidth;
    Temp_Data[4] = LoRaDevice.SpreadingFactor;
    Temp_Data[5] = 1;
    LOG_I(TAG, "%s: send beacon %04x\r\n", __func__, LoRaDevice.Slaver[i].shortAddress);
    CusProfile_Send(LoRaPacket.Rx_SAddr, Beacon, Temp_Data, 6, TRUE);
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
    if (LoRaDevice.NetMode != SearchForSlave)
        return;
    i = Compare_ShortAddr(LoRaPacket.Rx_SAddr);
    if (i == 0xFF)
    { // 没有发现匹配的设备
        CusProfile_Send(LoRaPacket.Rx_SAddr, Master_Request_Leave, NULL, 0, FALSE);
        return;
    }

    LoRaDevice.Slaver[i].RSSI = LoRaPacket.Rx_RSSI;
    LoRaAddSlaver(i);
    LOG_I(TAG, "Send DeviceAnnonce\r\n");
    LOG_I(TAG, "ID:%d SAddr:%04X Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, LoRaDevice.Slaver[i].shortAddress,
                LoRaDevice.Slaver[i].Mac[0], LoRaDevice.Slaver[i].Mac[1], LoRaDevice.Slaver[i].Mac[2], LoRaDevice.Slaver[i].Mac[3],
                LoRaDevice.Slaver[i].Mac[4], LoRaDevice.Slaver[i].Mac[5], LoRaDevice.Slaver[i].Mac[6], LoRaDevice.Slaver[i].Mac[7]);
    memset(infoStr, 0, 60);
    memcpy(infoStr, LoRaDevice.Self.Mac, 8);
    sprintf(&infoStr[8], "%s", LoRaDevice.Self.name);
    CusProfile_Send(LoRaPacket.Rx_SAddr, DeviceAnnonce, (uint8_t *)infoStr, strnlen(LoRaDevice.Self.name, 15) + 8, TRUE);

    // 将设备的信息格式化为字符串
    memset(macStr, 0, 24);
    memset(infoStr, 0, 60);
    MacToString(LoRaDevice.Slaver[i].Mac, macStr);
    sprintf(infoStr, "CON__%s__%s__%d", LoRaDevice.Slaver[i].name, macStr, LoRaDevice.Slaver[i].RSSI);
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
    i = Compare_ShortAddr(LoRaPacket.Rx_SAddr);
    LOG_I(TAG, "Send Master_Leave_ACK\r\n");
    CusProfile_Send(LoRaDevice.Slaver[i].shortAddress, Master_Leave_ACK, NULL, 0, TRUE);
    memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
    LoRaDelSlaver(i);
}

/**
 * Cmd_HeartBeat
 * @brief Reserved for a future function related to maintaining network connection (typically used for sending periodic signals to ensure active connections).
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_HeartBeat()
{
    uint8_t i;
    if(LoRaPacket.Rx_RSSI < Neighbor_MinRSSI)
        return;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Neighbor[i].shortAddress == LoRaPacket.Rx_SAddr)
        {
            LoRaDevice.Neighbor[i].RSSI = LoRaPacket.Rx_RSSI;
            LoRaDevice.Neighbor[i].timeout = Neighbor_Timeout;
            return;
        }
    }
    if (i == MAX_CHILDREN)
    {
        for(i = 0;i < MAX_CHILDREN;i++)
        {
            if(LoRaDevice.Neighbor[i].shortAddress == 0)
            {
                LoRaDevice.Neighbor[i].shortAddress = LoRaPacket.Rx_SAddr;
                LoRaDevice.Neighbor[i].RSSI = LoRaPacket.Rx_RSSI;
                LoRaDevice.Neighbor[i].timeout = Neighbor_Timeout;
                LOG_I(TAG, "Neighbor[%d] shortAddress:%04X RSSI:%d\r\n", i, LoRaDevice.Neighbor[i].shortAddress, LoRaDevice.Neighbor[i].RSSI);
                return;
            }
        }
    }
    LOG_I(TAG, "Neighbor is full\r\n");
}

/**
 * PCmd_Master_Request_Leave
 * @brief Function used by the master device to request a sub-device to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Master_Request_Leave(uint8_t ID)
{
    LOG_I(TAG, "Send Master_Request_Leave\r\n");
    CusProfile_Send(LoRaDevice.Slaver[ID].shortAddress, Master_Request_Leave, NULL, 0, FALSE);
    memset(&LoRaDevice.Slaver[ID], 0, sizeof(LoRa_Node_t)); // 将数据全部清除，之后接收到该短地址的数据会直接要求离网
    LoRaDelSlaver(ID);
}
