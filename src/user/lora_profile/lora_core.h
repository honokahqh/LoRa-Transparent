#ifndef __LORA_CORE_H
#define __LORA_CORE_H

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "APP.h"
#include "lora_flash.h"
#include "lora_user.h"
#include "lora_at_cmd.h"

#define Lora_Enabel 		 1 // �Ƿ���Lora����

#define Lora_Register_Enable 1	//�Ƿ���Loraע���б��ܣ������������ע��ʱ��Ҫ����һ��ע��ʹ��
#define Lora_Always_Master	 0  //�Ƿ�һֱ��Ϊ����
#define Lora_Always_Slaver   0  //�Ƿ�һֱ��Ϊ�ӻ�
#define Lora_Is_APP			 0 // �Ƿ�ΪAPP,���Ϊboot�������вü���Ч���

#define TRUE            1
#define FALSE           0

#define Device_Num_Max 			32

/* DeviceType */
#define Type_Master				0x77
#define Type_RoomCtrl			0x7B
#define Device_Type 			Type_RoomCtrl

#define	BoardCast				0xFFFE	//Panid:ȫ�ŵ��㲥  SAddr:ȫ���㲥

/* Public Cmd 0x01~0x9F*/	
#define BeaconRequest			0x01 	//���豸��������
#define Beacon					0x02 	//�����ظ����豸��������
#define	SlaverInNet				0x03	//�豸������(�ӻ��ظ�)
#define DeviceAnnonce			0x04	//�豸������(�����㲥)

#define Master_Request_Leave	0x11	//����Ҫ�����豸����(��ʱֱ��ɾ������)
#define	Slaver_Leave_ACK		0x12	//���豸�ظ�������(���豸ɾ����������)
#define Slaver_Request_Leave    0x13    //���豸Ҫ������(��ʱֱ��ɾ����������)
#define Master_Leave_ACK        0x14    //�����ظ����豸����Ҫ��(ɾ�����豸����)
#define Lora_Change_Para		0x15	//����Ҫ�����豸�ı�ͨѶ����
#define Lora_Query_RSSI			0x16	//����Ҫ�����豸��ѯ�ź�����
#define Lora_Query_RSSI_ACK		0x17	//����Ҫ�����豸��ѯ�ź�����

// �м���Ҫ�������豸SADDR-NAME,�ڵ���Ҫ�����м�SADDR-NAME
#define Lora_Find_Relay			0x18	//�豸�����м�
#define Lora_Find_Relay_ACK		0x19	//�豸�����м�ACK
#define Lora_Find_Point			0x1A	//�м̲����豸
#define Lora_Fine_Point_ACK 	0x1B	//�м̲����豸ACK

#define HeartBeat				0x20	//�豸����������

#define LoRa_SetSlaverName		0x30	//�����������豸����
#define LoRa_SetSlaverName_ACK	0x31	//�ӻ��ظ�����
#define LoRa_SetSelfName		0x32	//�豸�޸��Լ����ֵ�֪ͨ


#define Query_Version			0x40	//������ӻ��汾��
#define Query_Version_ACK		0x41	//�ӻ��ظ��汾��
#define Query_SubVersion		0x42	//���������豸�汾��
#define Query_SubVersion_ACK	0x43	//�ӻ��ظ�����
#define OTA_Device				0x51	//��������OTA����
#define OTA_Device_ACK			0x52	//�ӻ�ACK
#define OTA_SubDeVice			0x53	//����Ҫ��ӻ������豸OTA����
#define OTA_SubDevice_ACK		0x54	//�ӻ�ACK

#define MBS_Cmd_Request			0x60	//��������͸����MBSָ��
#define MBS_Cmd_ACK				0x61	//�ӻ�����͸����MBSָ��

#define Lora_Para_Set			0x70	//Lora��������
#define Lora_Para_Set_ACK		0x71	//Lora��������ACK

#define Lora_SendData			0x80	//͸��
#define Lora_SendData_ACK		0x81	//͸��

#define Lora_RelayEndPoint		0x82	//�м̽ڵ��յ��󶨵��ӽڵ����ݰ�,ͨ����������й㲥
#define Lora_RelayOthers		0x83	//�м��ڵ��յ��ǰ󶨽ڵ����ݰ���ͨ�����������ӽڵ�㲥

/* Private Cmd ��׺0xA0~0xEF*/
#define MBS_Switch_CMD			0xA0	//�������״̬�ϱ�
#define MBS_Switch_CMD_ACK		0xA1	//�������״̬�ϱ�ACK
#define MBS_Switch_Query		0XA2	//�������״̬��ѯ

#define MBS_IR_Detect_CMD		0xA4	//����1���״̬�ϱ�
#define MBS_IR_Detect2_CMD		0xA5	//����2���״̬�ϱ�

#define DeviceTimeout			7200	//�豸ͨѶ��ʱ  ��λ��
#define HeatBeat_Period			60		//���豸���������
#define Register_Timeout		30		//ע�ᳬʱʱ��
#define Leave_Timeout			5		//��������ʱ

/* ����״̬ net_state */
#define Net_NotJoin				0
#define Net_JoinGateWay			1
#define Net_Joining				2
#define Net_SearchSlave			1
#define Net_SearchMaster		2

/* ���ݸ�ʽƫ���� */
#define DevType_Addr			0
#define PanIDH_Addr				1
#define PanIDL_Addr				2
#define SAddrH_Addr				3
#define SAddrL_Addr				4
#define DAddrH_Addr				5
#define DAddrL_Addr				6
#define PackID_Addr				7
#define Cmd_Addr				8
#define Len_Addr				9
#define Data_Addr				10

#define Name_Size				16
// �豸����16�����豸��һ�����豸
typedef struct
{
	//��������
	uint16_t PanID;				//PanID
	uint16_t SAddrMaster;		//�����̵�ַ
	uint16_t SAddrSelf;			//�����̵�ַ
	int16_t RssiMaster;		//�����ź�����
	uint16_t chip_ID;			//chipID�ܺ�
	uint8_t Channel;			//ͨѶ�ŵ� 0~25 472~497
	uint8_t DeviceType;			//�豸����
	uint8_t Net_State;			//����״̬ 0:δ���� 1:�����յ��������󲢷��� 2:�������
	uint8_t Mac[8];
	uint8_t MasterMac[8];
	char MasterName[Name_Size];
	char SelfName[Name_Size];
	char RelayName[Name_Size];
	uint16_t RelaySAddr;
	//ͨѶ���ݰ�
	int16_t Rx_RSSI;					//�ź�����
	uint8_t Rx_Data[255];
	uint8_t Rx_DevType;
	uint16_t Rx_PanID;
	uint16_t Rx_SAddr;
	uint16_t Rx_DAddr;
	uint8_t Rx_CMD;
	uint8_t Rx_PID;
	uint16_t Rx_Len;
	uint8_t Tx_Data[255];
	uint8_t Tx_Len;

	//������
	uint8_t Wait_ACK;
	uint8_t ACK_Timeout;		//ACK��ʱ�����ط�
	uint8_t ErrTimes;

	uint8_t NetOpen;		//ע��ʱ��
}Lora_state_t;
extern Lora_state_t Lora_State;

typedef struct
{	
	uint16_t SAddr;         	//16λ����̵�ַ
	uint16_t chip_ID;			//chipID�ܺ�
	uint8_t DeviceType;         //�豸����
	uint8_t Net_State;			//����״̬ 0:δ���� 1:�����յ��������󲢷��� 2:�������
	uint8_t Mac[8];
	char Name[Name_Size];
    uint8_t Wait_ACK;           //�������ڵȴ���ACK
	uint8_t ACK_Timeout;		//ACK��ʱ�����ط�
	uint8_t ErrTimes;			//�������	
	int16_t RSSI;				//�ź�����	
	uint32_t Timeout;			//��ʱʱ��
}associated_devices_t;
extern associated_devices_t Associated_devices[Device_Num_Max];

typedef struct 
{
	uint16_t SAddr;         //16λ����̵�ַ
	uint16_t chip_ID;
	uint8_t DeviceType;
	int16_t RSSI;
	uint8_t Mac[8];
	uint8_t timeout;
	uint8_t Register_enable;
}register_device_t;
extern register_device_t Register_Device;

// ͨ��
void ChipID_Syn();// chipIDͬ��
void Random_Delay(void);// ����ӳ�
uint8_t XOR_Calculate(uint8_t *data,uint8_t len);//Lora���ݰ�XORУ��
void Lora_ReceiveData2State();// Lora��������ͬ��
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck);//lora���ݰ�����
void CusProfile_Receive();//���ݰ����մ���

uint8_t Get_IDLE_ID(void);//������ȡ����ID
uint8_t Compare_ShortAddr(uint16_t Short_Addr);//�����Աȶ̵�ַ�Ƿ����б���
uint8_t Compare_Register_SAddr(uint16_t Short_Addr);//�����Աȶ̵�ַ�Ƿ���ע���б���
uint8_t Compare_MAC(uint8_t *Mac);//�����Ա�MAC�Ƿ����б���
void Lora_DataRetrans_Enable(uint8_t ID,uint8_t Cmd);// Lora���ݰ���ʱ�ط�ʹ��
void Lora_DataRetrans_Disable(uint8_t ID);// Lora���ݰ���ʱ�ط���ֹ
int StringToMac(const char *str, uint8_t Mac[8]);
void MacToString(const uint8_t Mac[8], char *str);
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8]);

void Set_Sleeptime(uint8_t time);// �ӻ���������ʱ��
void IAP_Data_Re_Request();// �ӻ�����IAP���ݰ�

//����
void Cmd_BeaconRequest(void);
void PCmd_Beacon(void);
void Cmd_SlaverInNet(void);
void PCmd_DeviceAnnonce(void);
void Cmd_HeartBeat(void);

void Cmd_Slaver_Request_Leave(void);
void PCmd_Master_Request_Leave(uint8_t ID);

void PCmd_MBS_Cmd_Request(uint8_t *data,uint8_t len);
void Cmd_MBS_Cmd_ACK();

void Cmd_OTA_Device();
void Cmd_OTA_SubDevice();
void Cmd_Lora_SendData();
// �ӻ�
void PCMd_Beacon(void);
void Cmd_Beacon(void);
void PCmd_SlaverInNet(void);
void PCmd_BeaconRequest(void);
void Cmd_DeviceAnnonce(void);
void Cmd_Lora_Change_Para(void);
void PCmd_HeartBeat(void);

void Cmd_Master_Request_Leave(void);
void PCmd_Slaver_Request_Leave(void);

void PCmd_MBS_Cmd_ACK(uint8_t *data,uint8_t len);
void Cmd_MBS_Cmd_Request();

void Slaver_Period_1s(void);
void Master_Period_1s(void);

void PCmd_OTA_SubDeviceAck(uint8_t *data,uint8_t len);

void Cmd_Query_Version();
void Cmd_Query_SubVersion();
void PCmd_Query_SubVersion_ACK();
void Cmd_Query_Rssi();
void Lora_Sleep(void);
#endif

