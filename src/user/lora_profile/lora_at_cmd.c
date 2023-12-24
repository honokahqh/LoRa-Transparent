#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lora_core.h"

#define MAX_CMD_LEN 100
#define MAX_DATA_LEN 100

uint8_t str_tok(char *str, char *Delim, char *str2, char **newtok)
{
    uint8_t count = 0, len = 0, total;
    total = strlen(Delim);
    uint8_t str_len = strlen(str);

    for (uint8_t i = 0; i < str_len; i++)
    {
        if (str[i] == Delim[count])
            count++;
        else
            count = 0;
        len++;
        if (count == total)
        {
            *newtok = &str[i + 1];
            strncpy(str2, str, len - total);
            str2[len - total] = '\0'; // 添加空字符
            return len - total;
        }
    }
    strncpy(str2, str, len);
    str2[len] = '\0'; // 添加空字符
    return len;
}

typedef struct
{
    char command[MAX_CMD_LEN];

    char data[MAX_DATA_LEN];
} Command;
Command cmd;
Command parse_input(char *input)
{
    char *token;
    memset(&cmd, 0, sizeof(Command));
    str_tok(input, "__", cmd.command, &token);
    if (token != NULL)
        memcpy(cmd.data, token, MAX_DATA_LEN);
    else
        memset(cmd.data, 0, MAX_DATA_LEN);
    return cmd;
}

void handle_disconnect(Command cmd)
{
    uint8_t mac[8], id;
    Debug_B("Disconnecting MAC: %s\n", cmd.data);
    StringToMac(cmd.data, mac);
    id = Compare_MAC(mac);
    if (id != 0xFF)
    {
        PCmd_Master_Request_Leave(id);
        Lora_AsData_Del(id);
        memset(&Associated_devices[id], 0, sizeof(associated_devices_t));
    }
}

void handle_connect(Command cmd)
{
    Debug_B("Connecting MAC: %s\n", cmd.data);
    StringToMac(cmd.data, Register_Device.Mac);
}

void handle_setname(Command cmd)
{
    char *token, mac[24], name[20];
    uint8_t mac8[8], id = 0xFF;
    memset(mac, 0, sizeof(mac));
    memset(name, 0, sizeof(name));
    memset(mac8, 0, sizeof(mac8));
    str_tok(cmd.data, "__", mac, &token);
    str_tok(token, "__", name, NULL); // 更新这里
    Debug_B("Setting MAC: %s to new name: %s\n", mac, name);
    StringToMac(mac, mac8);
    id = Compare_MAC(mac8);
    PCmd_LoRa_SetSlaverName(name, id);
}

void handle_setSelfName(Command cmd)
{
	char BleBuffer[50];
    memset(BleBuffer, 0, sizeof(BleBuffer));
    PCmd_LoRa_SetSelfName(cmd.data);
	sprintf(BleBuffer, "AT+NAME=乐清%s\r\n", Lora_State.SelfName);
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    Debug_B("Setting Self Name: %s\n", Lora_State.SelfName);
	delay_ms(500);
	sprintf(BleBuffer, "AT+REBOOT=1\r\n");
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
}

void handle_bleconnect(Command cmd)
{
    char BleBuffer[50], macStr[24];
    delay_ms(100);
    ble_state.connect = 1;
    if(Lora_Para_AT.BandWidth == 2 && Lora_Para_AT.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__1__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    else if(Lora_Para_AT.BandWidth == 1 && Lora_Para_AT.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__2__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    else if(Lora_Para_AT.BandWidth == 0 && Lora_Para_AT.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__3__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    else if(Lora_Para_AT.BandWidth == 0 && Lora_Para_AT.SpreadingFactor == 8)
        sprintf(BleBuffer, "PARA__%d__4__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    else if(Lora_Para_AT.BandWidth == 0 && Lora_Para_AT.SpreadingFactor == 9)
        sprintf(BleBuffer, "PARA__%d__5__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    else
        sprintf(BleBuffer, "PARA__%d__Err__%s__%d__%d", Lora_Para_AT.NetOpen, Lora_State.SelfName, (int)Lora_Para_AT.UART_BAUD, Lora_Para_AT.channel);
    BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    if (Lora_Para_AT.SAddrMaster < BoardCast && Lora_Para_AT.SAddrMaster > 0) // master有效
    {
        memset(macStr, 0, sizeof(macStr));
        MacToString(Lora_State.MasterMac, macStr);
        sprintf(BleBuffer, "MASTER__%s__%s__%d", Lora_State.MasterName, macStr, Lora_State.RssiMaster);
        BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    }
    else
    {
        sprintf(BleBuffer, "MASTER__UnConnected__NULL__NULL");
        BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    }
    for (uint8_t i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr != 0 && Associated_devices[i].SAddr != 0xFFFF)
        {
            memset(macStr, 0, sizeof(macStr));
            MacToString(Associated_devices[i].Mac, macStr);
            sprintf(BleBuffer, "CON__%s__%s__%d", Associated_devices[i].Name, macStr, Associated_devices[i].RSSI);
            BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
        }
    }
}

void handle_bledisconn(Command cmd)
{
    ble_state.connect = 0;
}
void handle_blename(Command cmd)
{
    ble_state.connect = 0;
}
typedef void (*CommandHandler)(Command);

typedef struct
{
    char command[MAX_CMD_LEN];
    CommandHandler handler;
} CommandMap;

bool handle_command(Command cmd, CommandMap *cmd_map, int num_commands)
{
    for (int i = 0; i < num_commands; i++)
    {
        if (strncmp(cmd.command, cmd_map[i].command, strlen(cmd_map[i].command)) == 0)
        {
            cmd_map[i].handler(cmd);
            return true;
        }
    }
    return false;
}

CommandMap cmd_map[] = {
    {"AT+DISCONNECT", handle_disconnect},
    {"AT+CONNECT", handle_connect},
    {"AT+SETNAME", handle_setname},
    {"AT+SETSELFNAME", handle_setSelfName},
    {"AT+BLEConnect", handle_bleconnect},
    {"+DISCONN:", handle_bledisconn},
    {"+NAME:", handle_blename},
};

// AT Process用于外部控制Lora模块，通过串口发送AT指令，Lora模块接收到指令后，会调用对应的处理函数
// 如果将协议集成到设备，可以直接调parseATCommand(const char *input)，也可以自己处理
// 处理函数原型
typedef void (*AT_Handler)(int parameter);

Lora_Para_AT_t Lora_Para_AT, Lora_Para_AT_Last;

const char *AT_CommandList[] = {
    "AT+RST",      // 重启
    "AT+CHANNEL",  // 设置信道              重启生效
    "AT+AR",       // 设置带宽              重启生效
    "AT+TXPOWER",  // 设置发送功率          重启生效
    "AT+PANID",    // 设置PANID             重启生效
    "AT+SADDR",    // 设置短地址            重启生效
    "AT+NETOPEN",  // 开启网络              重启生效
    "AT+NETCLOSE", // 关闭网络              重启生效
    "AT+LEAVE",    // Endpoint离开网络      直接生效
    "AT+DELETE",   // 删除设备X             直接生效
    "AT+PRINT",    // 打印当前连接的设备短地址       直接生效 ID:0 SADDR:5577 MAC:1234567812345678
    "AT+BAUD",     // 设置串口波特率         重启生效
};

typedef struct
{
    uint8_t index;
    int parameter;
} AT_Command;

AT_Command parseATCommand(char *input)
{
    AT_Command result = {0xFF, -1};

    for (int i = 0; i < sizeof(AT_CommandList) / sizeof(AT_CommandList[0]); i++)
    {
        if (strncmp(input, AT_CommandList[i], strlen(AT_CommandList[i])) == 0)
        {
            result.index = i;
            // Debug_B("AT Index:%d\r\n",result.index);
            const char *param_str = input + strlen(AT_CommandList[i]);
            if (*param_str != '\0')
            {
                result.parameter = atoi(param_str);
            }
            break;
        }
    }
    return result;
}
extern uint16_t page1_offset;

void handleSend(uint8_t *data, uint8_t len)
{
    uint8_t cmd;
    uint16_t DAddr;
    cmd = data[0];
    switch (cmd)
    {
    case Lora_SendData:
        DAddr = data[1] << 8 | data[2];
        CusProfile_Send(DAddr, cmd, &data[3], len - 3, 1);
        Debug_B("AT+SEND\r\n");
        break;
    default:
        Debug_B("ERROR:No such device.\r\n");
        return;
    }
}

void handleSleep(int parameter)
{
    Debug_B("AT+SLEEP\r\n");
}

void handleRst(int parameter)
{
    uint8_t temp_data[8];
    memset(temp_data, 0xFF, 8);
    Lora_State_Save();
    Debug_B("AT+RST\r\n");
    delay_ms(100);
    system_reset();
}

void handleChannel(int parameter)
{
    if (parameter < 0 || parameter > 100)
    {
        Debug_B("ERROR:Invalid channel number.\r\n");
        return;
    }
    Debug_B("AT+CHANNEL:%d\r\n", parameter);
    Lora_Para_AT.channel = parameter;
}

void handleAR(int parameter)
{
    if (parameter < 1 || parameter > 5)
    {
        Debug_B("ERROR:Invalid bandwidth.\r\n");
        return;
    }
    switch (parameter)
    {
    case 1:
        Lora_Para_AT.BandWidth = 2;
        Lora_Para_AT.SpreadingFactor = 7;
        break;
    case 2:
        Lora_Para_AT.BandWidth = 1;
        Lora_Para_AT.SpreadingFactor = 7;
        break;
    case 3:
        Lora_Para_AT.BandWidth = 0;
        Lora_Para_AT.SpreadingFactor = 7;
        break;
    case 4:
        Lora_Para_AT.BandWidth = 0;
        Lora_Para_AT.SpreadingFactor = 8;
        break;
    case 5:
        Lora_Para_AT.BandWidth = 0;
        Lora_Para_AT.SpreadingFactor = 9;
        break;
    default:
        break;
    }
    Lora_State_Save();
    Lora_State_Data_Syn();
    // LoraReInit();
    system_reset();
    Debug_B("AT+AR:%d\r\n", parameter);

}

void handleTxpower(int parameter)
{
    if (parameter < 0 || parameter > 21)
    {
        Debug_B("ERROR:Invalid tx power.\r\n");
        return;
    }
    Debug_B("AT+TXPOWER:%d\r\n", parameter);
    // Lora_Para_AT.Power = parameter;
}

void handlePanid(int parameter)
{
    // 0xFFFE为组播ID、0xFFFF为广播ID
    if (parameter < 1 || parameter > 0xFFFD)
    {
        Debug_B("ERROR:Invalid panid number.\r\n");
        return;
    }
    Debug_B("AT+PANID:%04X\r\n", parameter);
    Lora_Para_AT.PanID = parameter;
}

void handleSaddr(int parameter)
{
    if (parameter < 1 || parameter > 0xFFFD)
    {
        Debug_B("ERROR:Invalid SADDR number.\r\n");
        return;
    }
    Debug_B("AT+SADDR:%04X\r\n", parameter);
    Lora_Para_AT.SAddrSelf = parameter;
}

void handleNetopen(int parameter)
{
    Debug_B("AT+NETOPEN:%d\r\n", parameter % 256);
    if (parameter == 1)
        Lora_Para_AT.NetOpen = 1;
    else if (parameter == 2)
        Lora_Para_AT.NetOpen = 2;
    else
        Lora_Para_AT.NetOpen = 0;
    Lora_State.NetOpen = Lora_Para_AT.NetOpen;
    Lora_State_Save();
    Lora_State_Data_Syn();
    LoraReInit();
    system_reset();
}

void handleNetclose(int parameter)
{
    Debug_B("AT+NETCLOSE\r\n");
    Lora_Para_AT.NetOpen = 0;
    Lora_State.NetOpen = Lora_Para_AT.NetOpen;
    Lora_State_Save();
    Lora_State_Data_Syn();
    // LoraReInit();
    system_reset();
}

void handleLeave(int parameter)
{
    Debug_B("AT+LEAVE\r\n");
    // 处理逻辑...
    PCmd_Slaver_Request_Leave();
}

void handleDelet(int parameter)
{
    if (parameter < 0 || parameter > Device_Num_Max)
    {
        Debug_B("Invalid delet number.\r\n");
        return;
    }
    Debug_B("AT+DELET:%d\r\n", parameter);
    PCmd_Master_Request_Leave(parameter);
    Lora_AsData_Del(parameter);
    memset(&Associated_devices[parameter], 0, sizeof(associated_devices_t));
}

void handlePrint(int parameter)
{
    Debug_B("AT+PRINT:");
    if (Lora_State.Net_State == 0)
    {
        Debug_B("NO Master");
    }
    else
    {
        Debug_B("SAddrMaster:%d\r\n", Lora_State.SAddrMaster);
    }
    Debug_B("SAddr:%d\r\n", Lora_State.SAddrSelf);
    Debug_B("PanID:%d\r\n", Lora_State.PanID);
    Debug_B("Channel:%d\r\n", Lora_State.Channel);
    Debug_B("BW:%d\r\n", Lora_Para_AT.BandWidth);
    Debug_B("SF:%d\r\n\r\n", Lora_Para_AT.SpreadingFactor);
    Debug_B("Slaver:\r\n");
    for (uint8_t i = 0; i < Device_Num_Max; i++)
    {
        if (Associated_devices[i].SAddr != 0 && Associated_devices[i].SAddr != 0xFFFF)
        {
            Debug_B("   ID:%d SAddr:%04X Mac:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, Associated_devices[i].SAddr,
                    Associated_devices[i].Mac[0], Associated_devices[i].Mac[1], Associated_devices[i].Mac[2], Associated_devices[i].Mac[3],
                    Associated_devices[i].Mac[4], Associated_devices[i].Mac[5], Associated_devices[i].Mac[6], Associated_devices[i].Mac[7]);
        }
    }
}

void handleBaud(int parameter)
{
    if(parameter == 9600)
    {
        Lora_Para_AT.UART_BAUD = 9600;
    }
    else if(parameter == 19200)
    {
        Lora_Para_AT.UART_BAUD = 19200;
    }
    else if(parameter == 38400)
    {
        Lora_Para_AT.UART_BAUD = 38400;
    }
    else if(parameter == 57600)
    {
        Lora_Para_AT.UART_BAUD = 57600;
    }
    else if(parameter == 115200)
    {
        Lora_Para_AT.UART_BAUD = 115200;
    }
    else
    {
        Debug_B("ERROR:Invalid baud rate.\r\n");
        return;
    }
    Debug_B("AT+BAUD:%d\r\n", parameter);
    Lora_State_Save();
    system_reset();
}
// 定义处理函数数组
AT_Handler AT_Handlers[] = {
    handleRst,
    handleChannel,
    handleAR,
    handleTxpower,
    handlePanid,
    handleSaddr,
    handleNetopen,
    handleNetclose,
    handleLeave,
    handleDelet,
    handlePrint,
    handleBaud,
};

void executeCommand(AT_Command parsed_command)
{
    if (parsed_command.index != 0xFF)
    {
        Debug_B("index:%d \r\n", parsed_command.index);
        AT_Handlers[parsed_command.index](parsed_command.parameter);
    }
}

uint8_t processATCommand(char *input)
{
    AT_Command command = parseATCommand(input);
    if (command.index == 0xFF)
    {
        Command cmd = parse_input(input);
        if (handle_command(cmd, cmd_map, sizeof(cmd_map) / sizeof(cmd_map[0])) == false)
        {   
            return 0;
        }
    }
    executeCommand(command);
    return 1;
}