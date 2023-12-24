#include "lora_core.h"

/**
 * Cmd_Beacon
 * @brief Sub-device received network access permission information
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Beacon()
{
    if (Lora_State.Rx_Data[Len_Addr] == 6)
    {
        Lora_State.SAddrMaster = (Lora_State.Rx_Data[SAddrH_Addr] << 8) + Lora_State.Rx_Data[SAddrL_Addr];
        Lora_State.SAddrSelf = (Lora_State.Rx_Data[Data_Addr] << 8) + Lora_State.Rx_Data[Data_Addr + 1];
        Lora_State.PanID = Lora_State.Rx_PanID;
        Lora_State.Channel = Lora_State.Rx_Data[Data_Addr + 2];
        Lora_Para_AT.BandWidth = Lora_State.Rx_Data[Data_Addr + 3];
        Lora_Para_AT.SpreadingFactor = Lora_State.Rx_Data[Data_Addr + 4];
        Lora_State.Net_State = 2; // 未入网 已建立连接
        Lora_State.Wait_ACK = SlaverInNet;
        PCmd_SlaverInNet();
    }
}

/**
 * Cmd_DeviceAnnonce
 * @brief Sub-device declaration after receiving the host's network reply.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_DeviceAnnonce()
{
    Lora_State.Net_State = Net_JoinGateWay;
    Lora_State.Wait_ACK = 0;
    Lora_Para_AT.channel = Lora_State.Channel;
    Lora_Para_AT.SAddrSelf = Lora_State.SAddrSelf;
    Lora_Para_AT.SAddrMaster = Lora_State.SAddrMaster;
    Lora_Para_AT.PanID = Lora_State.PanID;
    Lora_Para_AT.Net_State = Net_JoinGateWay;
    Lora_Para_AT.NetOpen = 0;
    memcpy(Lora_State.MasterMac, &Lora_State.Rx_Data[Data_Addr], 8);
    memset(Lora_State.MasterName, 0, 16);
    strncpy(Lora_State.MasterName,(char *)&Lora_State.Rx_Data[Data_Addr + 8], Lora_State.Rx_Data[Len_Addr] - 8);
    Lora_State.MasterName[sizeof(Lora_State.MasterName) - 1] = '\0';
    Lora_State_Save();
    Lora_State_Data_Syn();
    // LoraReInit();
    system_reset();
}

/**
 * Cmd_Master_Request_Leave
 * @brief Sub-device receives a request to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Master_Request_Leave()
{
    Lora_Para_AT.SAddrSelf = Lora_State.chip_ID;
    Lora_Para_AT.SAddrMaster = BoardCast;
    Lora_Para_AT.PanID = Lora_State.chip_ID;
    Lora_Para_AT.Net_State = 0;
    Lora_Para_AT.NetOpen = 0;
    Lora_State_Save();
    Lora_State_Data_Syn();
    // LoraReInit();
    system_reset();
}

/**
 * Cmd_Lora_Change_Para
 * @brief change lora parameter, such as channel, spreading factor, bandwidth
 * @author Honokahqh
 * @date 2023-10-07
 */
void Cmd_Lora_Change_Para()
{
    Lora_Para_AT.SAddrSelf = (Lora_State.Rx_Data[Data_Addr] << 8) + Lora_State.Rx_Data[Data_Addr + 1];
    Lora_Para_AT.PanID = (Lora_State.Rx_Data[Data_Addr + 2] << 8) + Lora_State.Rx_Data[Data_Addr + 3];
    Lora_Para_AT.channel = Lora_State.Rx_Data[Data_Addr + 4];
    Lora_Para_AT.BandWidth = Lora_State.Rx_Data[Data_Addr + 5];
    Lora_Para_AT.SpreadingFactor = Lora_State.Rx_Data[Data_Addr + 6];
    Lora_Para_AT.SAddrMaster = (Lora_State.Rx_Data[Data_Addr + 8] << 8) + Lora_State.Rx_Data[Data_Addr + 9];
    Lora_State_Save();
    Lora_State_Data_Syn();
    // LoraReInit();
    system_reset();
}

void Cmd_Query_Rssi()
{
    uint8_t temp[2];
    temp[0] = Lora_State.Rx_RSSI >> 8;
    temp[1] = Lora_State.Rx_RSSI;
    CusProfile_Send(Lora_State.SAddrMaster, Lora_Query_RSSI, temp, 2, FALSE);
}
/**
 * PCmd_BeaconRequest
 * @brief Sub-device actively requests to join the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_BeaconRequest()
{
    uint8_t Temp_Data[24];
    if(Lora_State.Net_State == Net_NotJoin && Lora_State.NetOpen == Net_SearchMaster){
		Lora_State.Wait_ACK = BeaconRequest;
        Lora_State.ErrTimes = 0;
        Lora_State.ACK_Timeout = 3;
        Lora_State.Net_State = Net_NotJoin; // 未入网 未建立连接
        memset(Temp_Data, 0, 24);
        memcpy(Temp_Data, Lora_State.Mac, 8);
        strncpy((char *)&Temp_Data[8], Lora_State.SelfName, 15);
        CusProfile_Send(BoardCast, BeaconRequest, Temp_Data, strnlen(Lora_State.SelfName, 15) + 8, FALSE);
	}
}

/**
 * PCmd_SlaverInNet
 * @brief Sub-device actively requests to join the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_SlaverInNet()
{
    uint8_t Temp_Data[24];
    memset(Temp_Data, 0, 24);
    memcpy(Temp_Data, Lora_State.Mac, 8);
    memcpy(&Temp_Data[8], Lora_State.SelfName, strnlen(Lora_State.SelfName, 16));
    CusProfile_Send(BoardCast, SlaverInNet, Temp_Data, strnlen(Lora_State.SelfName, 16) + 8, FALSE);
}

/**
 * PCmd_Slaver_Request_Leave
 * @brief Sub-device actively requests to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Slaver_Request_Leave()
{
    CusProfile_Send(Lora_State.SAddrMaster, Slaver_Request_Leave, NULL, 0, FALSE);
    Lora_Para_AT.SAddrSelf = Lora_State.chip_ID;
    Lora_Para_AT.SAddrMaster = 0;
    Lora_Para_AT.PanID = Lora_State.chip_ID;
    Lora_Para_AT.Net_State = 0;
    Lora_Para_AT.NetOpen = 0;
    Lora_State_Save();
}

/**
 * PCmd_HeartBeat
 * @brief Sub-device sends a heartbeat.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_HeartBeat()
{
    uint8_t temp[2];
    if (Lora_State.Net_State != Net_JoinGateWay)
        return;
    temp[0] = Device_Type;
    temp[1] = Dev_Version;
    CusProfile_Send(Lora_State.SAddrMaster, HeartBeat, temp, 2, FALSE);
}

