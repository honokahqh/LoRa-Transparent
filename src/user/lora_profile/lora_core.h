#ifndef __LORA_CORE_H
#define __LORA_CORE_H

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "APP.h"
#include "lora_flash.h"
#include "lora_user.h"
#include "lora_at_cmd.h"

#define Lora_Enabel 1 // �Ƿ���Lora����

#define Lora_Register_Enable 1 // �Ƿ���Loraע���б��ܣ������������ע��ʱ��Ҫ����һ��ע��ʹ��
#define Lora_Always_Master 0   // �Ƿ�һֱ��Ϊ����
#define Lora_Always_Slaver 0   // �Ƿ�һֱ��Ϊ�ӻ�
#define Lora_Is_APP 0		   // �Ƿ�ΪAPP,���Ϊboot�������вü���Ч���

#define TRUE 1
#define FALSE 0

/* DeviceType */
#define Type_Master 0x77
#define Type_RoomCtrl 0x7B
#define Device_Type Type_RoomCtrl

#define BoardCast 0xFFFE // Panid:ȫ�ŵ��㲥  SAddr:ȫ���㲥

/* Public Cmd 0x01~0x9F*/
#define BeaconRequest 0x01 // ���豸��������
#define Beacon 0x02		   // �����ظ����豸��������
#define SlaverInNet 0x03   // �豸������(�ӻ��ظ�)
#define DeviceAnnonce 0x04 // �豸������(�����㲥)

#define Master_Request_Leave 0x11 // ����Ҫ�����豸����(��ʱֱ��ɾ������)
#define Slaver_Leave_ACK 0x12	  // ���豸�ظ�������(���豸ɾ����������)
#define Slaver_Request_Leave 0x13 // ���豸Ҫ������(��ʱֱ��ɾ����������)
#define Master_Leave_ACK 0x14	  // �����ظ����豸����Ҫ��(ɾ�����豸����)
#define Lora_Change_Para 0x15	  // ����Ҫ�����豸�ı�ͨѶ����
#define Lora_Query_RSSI 0x16	  // ����Ҫ�����豸��ѯ�ź�����
#define Lora_Query_RSSI_ACK 0x17  // ����Ҫ�����豸��ѯ�ź�����

// �м���Ҫ�������豸SADDR-NAME,�ڵ���Ҫ�����м�SADDR-NAME
#define Lora_Bind_Parent 0x18	  // �豸�����м̸��ڵ�
#define Lora_Bind_Parent_ACK 0x19 // �豸�����м̸��ڵ�ACK
#define Lora_Bind_Child 0x1A	  // �豸�����м��ӽڵ�
#define Lora_Bind_Child_ACK 0x1B  // �豸�����м��ӽڵ�ACK

#define LoRa_Bind_Init 0x21		   // Դ�ڵ㷢����·��
#define LoRa_Bind_NbrReply 0x22	   // �м̽ڵ�ظ�-�ھӽڵ���Ŀ��ڵ�
#define LoRa_Bind_SrcAck 0x23	   // Դ�ڵ�ظ��м̽ڵ�-��֪
#define LoRa_Bind_RelayTry 0x24	   // �м̽ڵ㳢�Ժ�Ŀ��ڵ㽨����·
#define LoRa_Bind_TgtAck 0x25	   // Ŀ��ڵ�ظ��м̽ڵ�-��֪
#define LoRa_Bind_Success 0x26	   // �м̽ڵ�ظ�Դ�ڵ�-�ɹ�
#define LoRa_Bind_Success_ACK 0x27 // Դ�ڵ�ظ��м̽ڵ�-�ɹ�

#define HeartBeat 0x20 // �豸����������

#define LoRa_SetSlaverName 0x30		// �����������豸����
#define LoRa_SetSlaverName_ACK 0x31 // �ӻ��ظ�����
#define LoRa_SetSelfName 0x32		// �豸�޸��Լ����ֵ�֪ͨ

#define Query_Version 0x40		  // ������ӻ��汾��
#define Query_Version_ACK 0x41	  // �ӻ��ظ��汾��
#define Query_SubVersion 0x42	  // ���������豸�汾��
#define Query_SubVersion_ACK 0x43 // �ӻ��ظ�����
#define OTA_Device 0x51			  // ��������OTA����
#define OTA_Device_ACK 0x52		  // �ӻ�ACK
#define OTA_SubDeVice 0x53		  // ����Ҫ��ӻ������豸OTA����
#define OTA_SubDevice_ACK 0x54	  // �ӻ�ACK

#define MBS_Cmd_Request 0x60 // ��������͸����MBSָ��
#define MBS_Cmd_ACK 0x61	 // �ӻ�����͸����MBSָ��

#define Lora_Para_Set 0x70	   // Lora��������
#define Lora_Para_Set_ACK 0x71 // Lora��������ACK

#define Lora_SendData 0x80	   // ͸��
#define Lora_SendData_ACK 0x81 // ͸��

/* Private Cmd ��׺0xA0~0xEF*/
#define MBS_Switch_CMD 0xA0		// �������״̬�ϱ�
#define MBS_Switch_CMD_ACK 0xA1 // �������״̬�ϱ�ACK
#define MBS_Switch_Query 0XA2	// �������״̬��ѯ

#define MBS_IR_Detect_CMD 0xA4	// ����1���״̬�ϱ�
#define MBS_IR_Detect2_CMD 0xA5 // ����2���״̬�ϱ�

#define DeviceTimeout 7200	// �豸ͨѶ��ʱ  ��λ��
#define HeatBeat_Period 60	// ���豸���������
#define Register_Timeout 30 // ע�ᳬʱʱ��
#define Leave_Timeout 5		// ��������ʱ

/* ����״̬ net_state */
#define Net_NotJoin 0
#define Net_JoinGateWay 1
#define Net_Joining 2

/* ���ݸ�ʽƫ���� */
#define DevType_Addr 0
#define PanIDH_Addr 1
#define PanIDL_Addr 2
#define SAddrH_Addr 3
#define SAddrL_Addr 4
#define DAddrH_Addr 5
#define DAddrL_Addr 6
#define PackID_Addr 7
#define Cmd_Addr 8
#define Len_Addr 9
#define Data_Addr 10

#define MAX_CHILDREN 32
#define MAC_Size 8
#define Name_Size 16

#define Neighbor_Timeout 600 // �ھӽڵ㳬ʱʱ��
#define Neighbor_MinRSSI -75 // �ھӽڵ���С�ź�ǿ��
// ͨѶģʽ
typedef enum
{
	Normal = 0,			// ��������ģʽ
	SearchForSlave = 1, // ��ѰSlaveģʽ
	SearchForMaster = 2 // ��ѰMasterģʽ
} NetworkMode_t;

typedef enum
{
	NodeType_None = 0, // ���Ǹ��ڵ�Ҳ�����ӽڵ�
	NodeType_Parent,   // ���ڵ�
	NodeType_Child	   // �ӽڵ�
} NodeType;

// LoRa�ڵ�������
typedef struct
{
	uint16_t shortAddress; // 16λ�̵�ַ
	uint8_t Mac[MAC_Size]; // MAC��ַ
	char name[Name_Size];  // �ڵ����ƣ����15�ֽڣ�+1 ���ڴ洢�����ַ� '\0'
	uint8_t DevcieType;	   // �豸����
	int16_t RSSI;		   // �ź�����
	uint32_t timeout;	   // ���һ������ʱ��
} LoRa_Node_t;

// LoRa�м�����
typedef struct
{
	LoRa_Node_t A;
	LoRa_Node_t B;
	LoRa_Node_t Relay;
} LoRa_RelayGroup_t;


// LoRaͨѶ���ݰ�
typedef struct
{
	uint8_t Tx_Data[256];
	uint8_t Tx_Len;

	uint8_t Rx_Data[256];
	uint8_t Rx_Len;
	uint8_t Rx_DevType;
	uint16_t Rx_PanID;
	uint16_t Rx_SAddr;
	uint16_t Rx_DAddr;
	uint8_t Rx_CMD;
	uint8_t Rx_PID;
	int16_t Rx_RSSI; // �ź�����

	uint8_t Wait_ACK;
	uint8_t AckTimeout; // ACK��ʱ�����ط�
	uint8_t ErrTimes;
} CommunicationPacket_t;
extern CommunicationPacket_t LoRaPacket; // ͨѶ���ݰ�

typedef struct
{
	uint8_t power;
	uint8_t channel;
	uint8_t BandWidth;
	uint8_t SpreadingFactor;
	uint16_t PanID;
	uint16_t SAddr;
	uint32_t UART_Baud;
} LoRaBackup_t;
extern LoRaBackup_t LoRaBackup;
// �豸����16�����豸��һ�����豸
typedef struct
{
	// ͨѶ����
	uint8_t channel;		 // ͨ��
	uint8_t power;			 // ����
	uint8_t BandWidth;		 // ����
	uint8_t SpreadingFactor; // ��Ƶ����

	// �������
	uint16_t PanID;	   // PanID
	uint16_t chip_ID;  // MAC�ܺ�
	uint8_t Net_State; // ����״̬ 0:δ���� 1:����
	NetworkMode_t NetMode;

	LoRa_Node_t Self;				  // ������Ϣ
	LoRa_Node_t Master;				  // ������Ϣ
	LoRa_Node_t Slaver[MAX_CHILDREN]; // ���豸��Ϣ

	LoRa_Node_t Neighbor[MAX_CHILDREN];			// �ھӽڵ���Ϣ
	LoRa_RelayGroup_t RelayGroup[MAX_CHILDREN]; // �м�����Ϣ
} LoRaDevice_t;
extern LoRaDevice_t LoRaDevice; // �豸��Ϣ

typedef struct
{
	LoRa_Node_t device;
	uint8_t timeout;
	uint8_t Register_enable;
} LoRaRegister_t;
extern LoRaRegister_t RegisterDevice;

// core��
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck); // Lora���ݷ���
void CusProfile_Receive();																	  // Lora���ݽ��մ���-����Э��������ݲ�������Ӧ������������
void LoRa_Period_1s();																		  // 1s��ʱ��

// funcion��
uint8_t Get_IDLE_ID();										  // ��ȡSlaver�Ŀ���ID
uint8_t Compare_ShortAddr(uint16_t Short_Addr);				  // �Ա�Slaver�Ķ̵�ַ,ʧ�ܷ���0xFF
uint8_t Compare_MAC(uint8_t *Mac);							  // �Ա�Slaver��MAC��ַ,ʧ�ܷ���0xFF
uint8_t XOR_Calculate(uint8_t *data, uint8_t len);			  // ���У�����
void Lora_ReceiveData2State();								  // �������ݴ���������
int StringToMac(const char *str, uint8_t Mac[8]);			  // �ַ���תMAC��ַ
void MacToString(const uint8_t Mac[8], char *str);			  // MAC��ַת�ַ���
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8]); // �Ƚ�����MAC��ַ�Ƿ����
void Reset_LoRa();											  // ����Lora

// CMD_Slaver
void Cmd_Beacon();				  // �����ظ����豸��������
void Cmd_DeviceAnnonce();		  // �豸������(�����㲥)
void Cmd_Master_Request_Leave();  // ����Ҫ�����豸����(��ʱֱ��ɾ������)
void Cmd_Lora_Change_Para();	  // ����Ҫ�����豸�ı�ͨѶ����
void Cmd_Query_Rssi();			  // ����Ҫ�����豸��ѯ�ź�����
void PCmd_BeaconRequest();		  // ���豸��������
void PCmd_SlaverInNet();		  // �豸������(�ӻ��ظ�)
void PCmd_Slaver_Request_Leave(); // ���豸Ҫ������(��ʱֱ��ɾ����������)
void PCmd_HeartBeat();			  // �豸����������

// CMD_Master
void Cmd_BeaconRequest();					// ���豸��������
void Cmd_SlaverInNet();						// �豸������(�ӻ��ظ�)
void Cmd_Slaver_Request_Leave();			// ���豸Ҫ������(��ʱֱ��ɾ����������)
void Cmd_HeartBeat();						// �豸����������
void PCmd_Master_Request_Leave(uint8_t ID); // ����Ҫ�����豸����(��ʱֱ��ɾ������)

// user ��Ҫuser����оƬȥ�����/user���ж����
void ChipID_Syn();								 // оƬIDͬ��
void Lora_StateInit();							 // �豸��ʼ��
void User_Slaver_Cmd();							 // �Զ����ָ��
void Lora_Send_Data(uint8_t *data, uint8_t len); // ���ݷ���
void Lora_Sleep();								 // Lora����

// ��������&�м�
void Cmd_LoRa_SetSlaverName();						  // �ӻ��ظ�����
void PCmd_LoRa_SetSlaverName(char *name, uint8_t ID); // �����������豸����
void PCmd_Lora_Bind_Parent(char *name);				  // �豸�����м̸��ڵ�
void Cmd_Lora_Bind_Parent();						  // �豸�����м̸��ڵ�ACK
void Cmd_LoRa_Bind_Parent_ACK();					  // �豸�����м̸��ڵ�ACK
void PCmd_Lora_Bind_Child(char *name);				  // �豸�����м��ӽڵ�
void Cmd_Lora_Bind_Child();							  // �豸�����м��ӽڵ�ACK
void Cmd_LoRa_Bind_Child_ACK();						  // �豸�����м��ӽڵ�ACK

#endif
