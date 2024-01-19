#ifndef __LORA_CORE_H
#define __LORA_CORE_H

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "APP.h"
#include "lora_flash.h"
#include "lora_user.h"
#include "lora_at_cmd.h"

#define Lora_Enabel 1 // 是否开启Lora功能

#define Lora_Register_Enable 1 // 是否开启Lora注册列表功能，如果开启则在注册时需要增加一步注册使能
#define Lora_Always_Master 0   // 是否一直作为主机
#define Lora_Always_Slaver 0   // 是否一直作为从机
#define Lora_Is_APP 0		   // 是否为APP,如果为boot可以自行裁剪无效组件

#define TRUE 1
#define FALSE 0

/* DeviceType */
#define Type_Master 0x77
#define Type_RoomCtrl 0x7B
#define Device_Type Type_RoomCtrl

#define BoardCast 0xFFFE // Panid:全信道广播  SAddr:全网广播

/* Public Cmd 0x01~0x9F*/
#define BeaconRequest 0x01 // 子设备请求入网
#define Beacon 0x02		   // 主机回复子设备入网请求
#define SlaverInNet 0x03   // 设备已入网(从机回复)
#define DeviceAnnonce 0x04 // 设备已入网(主机广播)

#define Master_Request_Leave 0x11 // 主机要求子设备离网(超时直接删除数据)
#define Slaver_Leave_ACK 0x12	  // 子设备回复已离网(子设备删除网络数据)
#define Slaver_Request_Leave 0x13 // 子设备要求离网(超时直接删除网络数据)
#define Master_Leave_ACK 0x14	  // 主机回复子设备离网要求(删除子设备数据)
#define Lora_Change_Para 0x15	  // 主机要求子设备改变通讯参数
#define Lora_Query_RSSI 0x16	  // 主机要求子设备查询信号质量
#define Lora_Query_RSSI_ACK 0x17  // 主机要求子设备查询信号质量

// 中继需要保存子设备SADDR-NAME,节点需要保存中继SADDR-NAME
#define Lora_Bind_Parent 0x18	  // 设备查找中继父节点
#define Lora_Bind_Parent_ACK 0x19 // 设备查找中继父节点ACK
#define Lora_Bind_Child 0x1A	  // 设备查找中继子节点
#define Lora_Bind_Child_ACK 0x1B  // 设备查找中继子节点ACK

#define LoRa_Bind_Init 0x21		   // 源节点发起链路绑定
#define LoRa_Bind_NbrReply 0x22	   // 中继节点回复-邻居节点有目标节点
#define LoRa_Bind_SrcAck 0x23	   // 源节点回复中继节点-已知
#define LoRa_Bind_RelayTry 0x24	   // 中继节点尝试和目标节点建立链路
#define LoRa_Bind_TgtAck 0x25	   // 目标节点回复中继节点-已知
#define LoRa_Bind_Success 0x26	   // 中继节点回复源节点-成功
#define LoRa_Bind_Success_ACK 0x27 // 源节点回复中继节点-成功

#define HeartBeat 0x20 // 设备在线心跳包

#define LoRa_SetSlaverName 0x30		// 主机设置子设备名称
#define LoRa_SetSlaverName_ACK 0x31 // 从机回复主机
#define LoRa_SetSelfName 0x32		// 设备修改自己名字的通知

#define Query_Version 0x40		  // 主机查从机版本号
#define Query_Version_ACK 0x41	  // 从机回复版本号
#define Query_SubVersion 0x42	  // 主机查子设备版本号
#define Query_SubVersion_ACK 0x43 // 从机回复主机
#define OTA_Device 0x51			  // 主机发送OTA更新
#define OTA_Device_ACK 0x52		  // 从机ACK
#define OTA_SubDeVice 0x53		  // 主机要求从机的子设备OTA更新
#define OTA_SubDevice_ACK 0x54	  // 从机ACK

#define MBS_Cmd_Request 0x60 // 主机向下透传的MBS指令
#define MBS_Cmd_ACK 0x61	 // 从机向上透传的MBS指令

#define Lora_Para_Set 0x70	   // Lora参数设置
#define Lora_Para_Set_ACK 0x71 // Lora参数设置ACK

#define Lora_SendData 0x80	   // 透传
#define Lora_SendData_ACK 0x81 // 透传

/* Private Cmd 后缀0xA0~0xEF*/
#define MBS_Switch_CMD 0xA0		// 开关面板状态上报
#define MBS_Switch_CMD_ACK 0xA1 // 开关面板状态上报ACK
#define MBS_Switch_Query 0XA2	// 开关面板状态查询

#define MBS_IR_Detect_CMD 0xA4	// 红外1检测状态上报
#define MBS_IR_Detect2_CMD 0xA5 // 红外2检测状态上报

#define DeviceTimeout 7200	// 设备通讯超时  单位秒
#define HeatBeat_Period 60	// 子设备心跳包间隔
#define Register_Timeout 30 // 注册超时时间
#define Leave_Timeout 5		// 离网倒计时

/* 网络状态 net_state */
#define Net_NotJoin 0
#define Net_JoinGateWay 1
#define Net_Joining 2

/* 数据格式偏移量 */
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

#define Neighbor_Timeout 600 // 邻居节点超时时间
#define Neighbor_MinRSSI -75 // 邻居节点最小信号强度
// 通讯模式
typedef enum
{
	Normal = 0,			// 正常工作模式
	SearchForSlave = 1, // 搜寻Slave模式
	SearchForMaster = 2 // 搜寻Master模式
} NetworkMode_t;

typedef enum
{
	NodeType_None = 0, // 不是父节点也不是子节点
	NodeType_Parent,   // 父节点
	NodeType_Child	   // 子节点
} NodeType;

// LoRa节点描述符
typedef struct
{
	uint16_t shortAddress; // 16位短地址
	uint8_t Mac[MAC_Size]; // MAC地址
	char name[Name_Size];  // 节点名称，最多15字节，+1 用于存储结束字符 '\0'
	uint8_t DevcieType;	   // 设备类型
	int16_t RSSI;		   // 信号质量
	uint32_t timeout;	   // 最后一次心跳时间
} LoRa_Node_t;

// LoRa中继数据
typedef struct
{
	LoRa_Node_t A;
	LoRa_Node_t B;
	LoRa_Node_t Relay;
} LoRa_RelayGroup_t;


// LoRa通讯数据包
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
	int16_t Rx_RSSI; // 信号质量

	uint8_t Wait_ACK;
	uint8_t AckTimeout; // ACK超时数据重发
	uint8_t ErrTimes;
} CommunicationPacket_t;
extern CommunicationPacket_t LoRaPacket; // 通讯数据包

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
// 设备最多绑定16个子设备，一个主设备
typedef struct
{
	// 通讯参数
	uint8_t channel;		 // 通道
	uint8_t power;			 // 功率
	uint8_t BandWidth;		 // 带宽
	uint8_t SpreadingFactor; // 扩频因子

	// 网络参数
	uint16_t PanID;	   // PanID
	uint16_t chip_ID;  // MAC总和
	uint8_t Net_State; // 网络状态 0:未入网 1:入网
	NetworkMode_t NetMode;

	LoRa_Node_t Self;				  // 自身信息
	LoRa_Node_t Master;				  // 主机信息
	LoRa_Node_t Slaver[MAX_CHILDREN]; // 子设备信息

	LoRa_Node_t Neighbor[MAX_CHILDREN];			// 邻居节点信息
	LoRa_RelayGroup_t RelayGroup[MAX_CHILDREN]; // 中继组信息
} LoRaDevice_t;
extern LoRaDevice_t LoRaDevice; // 设备信息

typedef struct
{
	LoRa_Node_t device;
	uint8_t timeout;
	uint8_t Register_enable;
} LoRaRegister_t;
extern LoRaRegister_t RegisterDevice;

// core内
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck); // Lora数据发送
void CusProfile_Receive();																	  // Lora数据接收处理-根据协议解析数据并调用相应函数处理数据
void LoRa_Period_1s();																		  // 1s定时器

// funcion内
uint8_t Get_IDLE_ID();										  // 获取Slaver的空闲ID
uint8_t Compare_ShortAddr(uint16_t Short_Addr);				  // 对比Slaver的短地址,失败返回0xFF
uint8_t Compare_MAC(uint8_t *Mac);							  // 对比Slaver的MAC地址,失败返回0xFF
uint8_t XOR_Calculate(uint8_t *data, uint8_t len);			  // 异或校验计算
void Lora_ReceiveData2State();								  // 接收数据存入各类参数
int StringToMac(const char *str, uint8_t Mac[8]);			  // 字符串转MAC地址
void MacToString(const uint8_t Mac[8], char *str);			  // MAC地址转字符串
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8]); // 比较两个MAC地址是否相等
void Reset_LoRa();											  // 重置Lora

// CMD_Slaver
void Cmd_Beacon();				  // 主机回复子设备入网请求
void Cmd_DeviceAnnonce();		  // 设备已入网(主机广播)
void Cmd_Master_Request_Leave();  // 主机要求子设备离网(超时直接删除数据)
void Cmd_Lora_Change_Para();	  // 主机要求子设备改变通讯参数
void Cmd_Query_Rssi();			  // 主机要求子设备查询信号质量
void PCmd_BeaconRequest();		  // 子设备请求入网
void PCmd_SlaverInNet();		  // 设备已入网(从机回复)
void PCmd_Slaver_Request_Leave(); // 子设备要求离网(超时直接删除网络数据)
void PCmd_HeartBeat();			  // 设备在线心跳包

// CMD_Master
void Cmd_BeaconRequest();					// 子设备请求入网
void Cmd_SlaverInNet();						// 设备已入网(从机回复)
void Cmd_Slaver_Request_Leave();			// 子设备要求离网(超时直接删除网络数据)
void Cmd_HeartBeat();						// 设备在线心跳包
void PCmd_Master_Request_Leave(uint8_t ID); // 主机要求子设备离网(超时直接删除数据)

// user 需要user根据芯片去定义的/user自行定义的
void ChipID_Syn();								 // 芯片ID同步
void Lora_StateInit();							 // 设备初始化
void User_Slaver_Cmd();							 // 自定义的指令
void Lora_Send_Data(uint8_t *data, uint8_t len); // 数据发送
void Lora_Sleep();								 // Lora休眠

// 名称设置&中继
void Cmd_LoRa_SetSlaverName();						  // 从机回复主机
void PCmd_LoRa_SetSlaverName(char *name, uint8_t ID); // 主机设置子设备名称
void PCmd_Lora_Bind_Parent(char *name);				  // 设备查找中继父节点
void Cmd_Lora_Bind_Parent();						  // 设备查找中继父节点ACK
void Cmd_LoRa_Bind_Parent_ACK();					  // 设备查找中继父节点ACK
void PCmd_Lora_Bind_Child(char *name);				  // 设备查找中继子节点
void Cmd_Lora_Bind_Child();							  // 设备查找中继子节点ACK
void Cmd_LoRa_Bind_Child_ACK();						  // 设备查找中继子节点ACK

#endif
