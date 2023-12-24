#ifndef __LORA_USER_H
#define __LORA_USER_H

#include "lora_core.h"


void Lora_StateInit();
void User_Slaver_Cmd();

void Lora_Send_Data(uint8_t *data,uint8_t len);

void PCmd_Call_Serivce();
void Wait2TXEnd();
void Wait2RXEnd();

void PCmd_LoRa_SetSlaverName(char *name, uint8_t ID);
void Cmd_LoRa_SetSlaverName();
void PCmd_LoRa_SetSelfName(char *name);
void Cmd_LoRa_SetSelfName();
#endif
