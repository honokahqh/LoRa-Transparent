#include "lora_core.h"

associated_devices_t Associated_devices[Device_Num_Max],Associated_Master;
register_device_t Register_Device;
Lora_state_t Lora_State;

/**
 * CusProfile_Send
 * @brief Lora���ݷ���
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t PackageID;//�������͵�Package��ACK��Package
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck)
{
	uint8_t i;
	Lora_State.Tx_Data[DevType_Addr] = Device_Type;//����
	Lora_State.Tx_Data[PanIDH_Addr] = Lora_State.PanID>>8;//��������
    Lora_State.Tx_Data[PanIDL_Addr] = Lora_State.PanID;//��������
    Lora_State.Tx_Data[SAddrH_Addr] = Lora_State.SAddrSelf>>8;//���������ַ
    Lora_State.Tx_Data[SAddrL_Addr] = Lora_State.SAddrSelf;//���������ַ
	Lora_State.Tx_Data[DAddrH_Addr] = DAddr>>8;//Ŀ�������ַ
    Lora_State.Tx_Data[DAddrL_Addr] = DAddr;//Ŀ�������ַ
    if(isAck){
        Lora_State.Tx_Data[PackID_Addr] = Lora_State.Rx_PID;
	}
    else{
        Lora_State.Tx_Data[PackID_Addr] = PackageID;
        PackageID++;
	}
	Lora_State.Tx_Data[Cmd_Addr] = cmd;
	Lora_State.Tx_Data[Len_Addr] = len;
	for(i = 0;i < len;i++)
		Lora_State.Tx_Data[i + Data_Addr] = *data++;
	Lora_State.Tx_Data[len + Data_Addr] = XOR_Calculate(Lora_State.Tx_Data, len + Data_Addr);
	Lora_State.Tx_Len = len + Data_Addr + 1 ;
    
	Debug_B("Send:");
    for (i = 0; i < Lora_State.Tx_Len; i++)
        Debug_B("%02X ", Lora_State.Tx_Data[i]);
    Debug_B("\r\n");
    Lora_Send_Data(Lora_State.Tx_Data,Lora_State.Tx_Len);
}

/**
 * CusProfile_Receive
 * @brief Lora���ݽ��մ���-����Э��������ݲ�������Ӧ������������
 * @author Honokahqh
 * @date 2023-08-05
 */
void CusProfile_Receive()
{
    Lora_ReceiveData2State();
    Debug_B("RSSI:%d Receive:",Lora_State.Rx_RSSI);
    for (uint16_t i = 0; i < Lora_State.Rx_Len; i++)
        Debug_B("%02X ", Lora_State.Rx_Data[i]);
    Debug_B("\r\n");
    /* ���У���Ƿ�һ�� */
    if(XOR_Calculate(Lora_State.Rx_Data, Lora_State.Rx_Len - 1) !=  Lora_State.Rx_Data[Lora_State.Rx_Len -1])
	{
		return;  
	}	
    /* ��֤PanID */
	if(Lora_State.Rx_PanID != Lora_State.PanID && Lora_State.Rx_PanID != BoardCast && Lora_State.PanID != BoardCast)
	{
		return;  
	}	  
    /* �����յ�BeaconRequest����ע��ģʽʱ�������̵�ַ��֤ step2 */
    if(Lora_State.Rx_CMD == BeaconRequest && Lora_State.NetOpen == Net_SearchSlave)
	{
		goto DataProcess;
	}   
	/* ��֤Ŀ�Ķ̵�ַ Ϊ�㲥�������ַ */
	if(Lora_State.Rx_DAddr != Lora_State.SAddrSelf && Lora_State.Rx_DAddr != BoardCast)
    {
		return;  
	}
#if Lora_Register_Enable
    /* ע��flagʹ��ʱ,��ע���豸��Ϣ������Register_Device�ڶ���AS�� */
    if (Compare_Register_SAddr(Lora_State.Rx_SAddr) != 0xFF && Lora_State.Rx_CMD == SlaverInNet)//��ע���б���
        goto DataProcess;
#endif
    DataProcess:
    switch(Lora_State.Rx_CMD)
    {
    case BeaconRequest:
        Cmd_BeaconRequest();
        break;
    case Beacon:
        Cmd_Beacon();
        break;
    case SlaverInNet:
        Cmd_SlaverInNet();
        break;
    case DeviceAnnonce:
        Cmd_DeviceAnnonce();
        break;
    case Master_Request_Leave:
        Cmd_Master_Request_Leave();
        break;
    case Slaver_Request_Leave:
        Cmd_Slaver_Request_Leave();
        break;
    case Lora_Change_Para:
        Cmd_Lora_Change_Para();
        break;
    case HeartBeat:
        Cmd_HeartBeat();
        break;
    case Lora_SendData:
		Cmd_Lora_SendData();
		break;
    case LoRa_SetSlaverName:
        Cmd_LoRa_SetSlaverName();
        break;
    case LoRa_SetSelfName:
        Cmd_LoRa_SetSelfName();
        break;
    default:
        User_Slaver_Cmd();
        break;
    }

}

/**
 * Slaver_Period_1s
 * @brief �ӻ�ÿ��״̬���´���
 * @author Honokahqh
 * @date 2023-08-05
 */
void Slaver_Period_1s()
{
    /* δ����״̬�������� */
	if(Lora_State.Wait_ACK == 0 && Lora_State.Net_State == Net_NotJoin && Lora_State.NetOpen == Net_SearchMaster){
		PCmd_BeaconRequest();
	}

    /* ACK��ʱ */
    if(Lora_State.ACK_Timeout){
        Lora_State.ACK_Timeout--;
        if(Lora_State.Wait_ACK && Lora_State.ACK_Timeout == 0){

            Lora_State.ErrTimes++;
            if(Lora_State.ErrTimes > 1){
                switch (Lora_State.Wait_ACK)
                {
                case BeaconRequest://����ʧ��
                    Lora_State.ErrTimes = 0;//���ô��� ��������
                    break;
                case SlaverInNet://����ʧ��
                    Lora_State.Net_State = 0;
                    break;
                default:
                    Lora_State.Wait_ACK = 0;
                    Lora_State.ErrTimes = 0;
                    break;
                }
            }
            switch (Lora_State.Wait_ACK)
            {
            case BeaconRequest:
                PCmd_BeaconRequest();
                Lora_State.ACK_Timeout = 3;
                break;
            case SlaverInNet:
                PCmd_SlaverInNet();
                Lora_State.ACK_Timeout = 3;
                break;
            }
        }
    }
}

/**
 * Master_Period_1s
 * @brief ����ÿ��״̬���´���
 * @author Honokahqh
 * @date 2023-08-05
 */
void Master_Period_1s()
{
#if Lora_Register_Enable
    if (Register_Device.timeout)
        Register_Device.timeout--;
    if (Register_Device.timeout == 0)
        memset(&Register_Device, 0, sizeof(register_device_t));
#endif
    for (uint8_t i = 0; i < Device_Num_Max; i++)
    { // ע���豸��ʱ���
        if (Associated_devices[i].Timeout)
			Associated_devices[i].Timeout--;
		if (Associated_devices[i].Timeout == 0 && Associated_devices[i].Net_State == Net_Joining)
			memset(&Associated_devices[i], 0, sizeof(associated_devices_t));
    }
}
