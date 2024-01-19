#ifndef __LORA_FLASH_H__
#define __LORA_FLASH_H__

#include "APP.h"
      
#define CM4_IRQ_VECT_BASE           0xE000ED08
#define CM4_IRQ_EN                  0xE000E100
#define CM4_IRQ_CLR                 0xE000E180

#define FLASH_START_ADDR            0x08000000
#define FLASH_MAX_SIZE              0x20000 //128KB

#define FlashData1_ADDR              0x0800E000
#define FlashData2_ADDR              0x0800F000 

// page 1 存储类型
#define Type_Lora_net 0x01
#define Type_Lora_net_len 16

#define Type_Lora_SelfName 0x02
#define Type_Lora_SelfName_len Name_Size

#define Type_Lora_MasterName 0x03
#define Type_Lora_MasterName_len Name_Size

// page 3
#define Type_Lora_Parent 0x04
#define Type_Lora_Parent_len Name_Size + 2

#define Type_Lora_Child 0x05
#define Type_Lora_Child_len Name_Size + 2

// page 2
#define Type_Lora_Slaver        0x06
#define Type_Lora_Slaver_len    32

void LoRa_NetPara_Save(uint8_t type); // page1相关数据存储

void LoRaAddSlaver(uint8_t ID); // page2相关数据存储
void LoRaDelSlaver(uint8_t ID); // page2相关数据存储

void LoRaAddChild(uint8_t ID);  // page3相关数据存储
void LoRaDelChild(uint8_t ID);  // page3相关数据存储
void LoRaAddParent();           // page3相关数据存储
void LoRaDelParent();           // page3相关数据存储

void LoRaFlashdataSyn();

void mbs_data_syn();
void mbs_data_save();

#endif

