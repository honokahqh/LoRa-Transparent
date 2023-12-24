#ifndef __LORA_CORE_H
#define __LORA_CORE_H

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "APP.h"
#include "lora_flash.h"
#include "lora_user.h"
#include "lora_at_cmd.h"

#define Lora_Enabel 		 1 // 是否开启Lora功能

#define Lora_Register_Enable 1	//是否开启Lora注册列表功能，如果开启则在注册时需要增加一步注册使能
#define Lora_Always_Master	 0  //是否一直作为主机
#define Lora_Always_Slaver   0  //是否一直作为从机
#define Lora_Is_APP			 0 // 是否为APP,如果为boot可以自行裁剪无效组件

#define TRUE            1
#define FALSE           0

#define Device_Num_Max 			32

/* DeviceType */
#define Type_Master				0x77
#define Type_RoomCtrl			0x7B
#define Device_Type 			Type_RoomCtrl

#define	BoardCast				0xFFFE	//Panid:全信道广播  SAddr:全网广播

/* Public Cmd 0x01~0x9F*/	
#define BeaconRequest			0x01 	//子设备请求入网
#define Beacon					0x02 	//主机回复子设备入网请求
#define	SlaverInNet				0x03	//设备已入网(从机回复)
#define DeviceAnnonce			0x04	//设备已入网(主机广播)

#define Master_Request_Leave	0x11	//主机要求子设备离网(超时直接删除数据)
#define	Slaver_Leave_ACK		0x12	//子设备回复已离网(子设备删除网络数据)
#define Slaver_Request_Leave    0x13    //子设备要求离网(超时直接删除网络数据)
#define Master_Leave_ACK        0x14    //主机回复子设备离网要求(删除子设备数据)
#define Lora_Change_Para		0x15	//主机要求子设备改变通讯参数
#define Lora_Query_RSSI			0x16	//主机要求子设备查询信号质量
#define Lora_Query_RSSI_ACK		0x17	//主机要求子设备查询信号质量

// 中继需要保存子设备SADDR-NAME,节点需要保存中继SADDR-NAME
#define Lora_Find_Relay			0x18	//设备查找中继
#define Lora_Find_Relay_ACK		0x19	//设备查找中继ACK
#define Lora_Find_Point			0x1A	//中继查找设备
#define Lora_Fine_Point_ACK 	0x1B	//中继查找设备ACK

#define HeartBeat				0x20	//设备在线心跳包

#define LoRa_SetSlaverName		0x30	//主机设置子设备名称
#define LoRa_SetSlaverName_ACK	0x31	//从机回复主机
#define LoRa_SetSelfName		0x32	//设备修改自己名字的通知


#define Query_Version			0x40	//主机查从机版本号
#define Query_Version_ACK		0x41	//从机回复版本号
#define Query_SubVersion		0x42	//主机查子设备版本号
#define Query_SubVersion_ACK	0x43	//从机回复主机
#define OTA_Device				0x51	//主机发送OTA更新
#define OTA_Device_ACK			0x52	//从机ACK
#define OTA_SubDeVice			0x53	//主机要求从机的子设备OTA更新
#define OTA_SubDevice_ACK		0x54	//从机ACK

#define MBS_Cmd_Request			0x60	//主机向下透传的MBS指令
#define MBS_Cmd_ACK				0x61	//从机向上透传的MBS指令

#define Lora_Para_Set			0x70	//Lora参数设置
#define Lora_Para_Set_ACK		0x71	//Lora参数设置ACK

#define Lora_SendData			0x80	//透传
#define Lora_SendData_ACK		0x81	//透传

#define Lora_RelayEndPoint		0x82	//中继节点收到绑定的子节点数据包,通过该命令进行广播
#define Lora_RelayOthers		0x83	//中级节点收到非绑定节点数据包，通过该命令向子节点广播

/* Private Cmd 后缀0xA0~0xEF*/
#define MBS_Switch_CMD			0xA0	//开关面板状态上报
#define MBS_Switch_CMD_ACK		0xA1	//开关面板状态上报ACK
#define MBS_Switch_Query		0XA2	//开关面板状态查询

#define MBS_IR_Detect_CMD		0xA4	//红外1检测状态上报
#define MBS_IR_Detect2_CMD		0xA5	//红外2检测状态上报

#define DeviceTimeout			7200	//设备通讯超时  单位秒
#define HeatBeat_Period			60		//子设备心跳包间隔
#define Register_Timeout		30		//注册超时时间
#define Leave_Timeout			5		//离网倒计时

/* 网络状态 net_state */
#define Net_NotJoin				0
#define Net_JoinGateWay			1
#define Net_Joining				2
#define Net_SearchSlave			1
#define Net_SearchMaster		2

/* 数据格式偏移量 */
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
// 设备最多绑定16个子设备，一个主设备
typedef struct
{
	//基本参数
	uint16_t PanID;				//PanID
	uint16_t SAddrMaster;		//主机短地址
	uint16_t SAddrSelf;			//本机短地址
	int16_t RssiMaster;		//主机信号质量
	uint16_t chip_ID;			//chipID总和
	uint8_t Channel;			//通讯信道 0~25 472~497
	uint8_t DeviceType;			//设备类型
	uint8_t Net_State;			//网络状态 0:未入网 1:主机收到入网请求并返回 2:入网完毕
	uint8_t Mac[8];
	uint8_t MasterMac[8];
	char MasterName[Name_Size];
	char SelfName[Name_Size];
	char RelayName[Name_Size];
	uint16_t RelaySAddr;
	//通讯数据包
	int16_t Rx_RSSI;					//信号质量
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

	//对主机
	uint8_t Wait_ACK;
	uint8_t ACK_Timeout;		//ACK超时数据重发
	uint8_t ErrTimes;

	uint8_t NetOpen;		//注册时间
}Lora_state_t;
extern Lora_state_t Lora_State;

typedef struct
{	
	uint16_t SAddr;         	//16位网络短地址
	uint16_t chip_ID;			//chipID总和
	uint8_t DeviceType;         //设备类型
	uint8_t Net_State;			//网络状态 0:未入网 1:主机收到入网请求并返回 2:入网完毕
	uint8_t Mac[8];
	char Name[Name_Size];
    uint8_t Wait_ACK;           //主机正在等待的ACK
	uint8_t ACK_Timeout;		//ACK超时数据重发
	uint8_t ErrTimes;			//错误次数	
	int16_t RSSI;				//信号质量	
	uint32_t Timeout;			//超时时间
}associated_devices_t;
extern associated_devices_t Associated_devices[Device_Num_Max];

typedef struct 
{
	uint16_t SAddr;         //16位网络短地址
	uint16_t chip_ID;
	uint8_t DeviceType;
	int16_t RSSI;
	uint8_t Mac[8];
	uint8_t timeout;
	uint8_t Register_enable;
}register_device_t;
extern register_device_t Register_Device;

// 通用
void ChipID_Syn();// chipID同步
void Random_Delay(void);// 随机延迟
uint8_t XOR_Calculate(uint8_t *data,uint8_t len);//Lora数据包XOR校验
void Lora_ReceiveData2State();// Lora接受数据同步
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck);//lora数据包发送
void CusProfile_Receive();//数据包接收处理

uint8_t Get_IDLE_ID(void);//主机获取空闲ID
uint8_t Compare_ShortAddr(uint16_t Short_Addr);//主机对比短地址是否在列表内
uint8_t Compare_Register_SAddr(uint16_t Short_Addr);//主机对比短地址是否在注册列表内
uint8_t Compare_MAC(uint8_t *Mac);//主机对比MAC是否在列表内
void Lora_DataRetrans_Enable(uint8_t ID,uint8_t Cmd);// Lora数据包超时重发使能
void Lora_DataRetrans_Disable(uint8_t ID);// Lora数据包超时重发禁止
int StringToMac(const char *str, uint8_t Mac[8]);
void MacToString(const uint8_t Mac[8], char *str);
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8]);

void Set_Sleeptime(uint8_t time);// 从机设置休眠时间
void IAP_Data_Re_Request();// 从机请求IAP数据包

//主机
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
// 从机
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

