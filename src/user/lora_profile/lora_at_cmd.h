#ifndef AT_PROCESS_H
#define AT_PROCESS_H
#include "lora_core.h"
#include "lora_user.h"

//��Ҫ�����ȫ������
//LoraӲ������:Ƶ��,����,����,��Ƶ����
//Э�����(ͨ��):�������,�豸����,�豸��ַ,�豸PANID
//Э�����(����):����ʱ��,�����豸�б�(MAC+SADDR)
//Э�����(�ӻ�):�Ƿ�����

typedef struct
{
    uint8_t channel;
    uint8_t Power;
    uint8_t BandWidth;
    uint8_t SpreadingFactor;

    uint16_t PanID;
    uint16_t SAddrMaster;
    uint16_t SAddrSelf;

    uint8_t NetOpen;//��������ʱ��
    uint8_t Net_State;

    uint32_t UART_BAUD; //���ڲ�����
}Lora_Para_AT_t;
extern Lora_Para_AT_t Lora_Para_AT,Lora_Para_AT_Last;
uint8_t processATCommand(char *input);
void handleSend(uint8_t *data, uint8_t len);
#endif //
