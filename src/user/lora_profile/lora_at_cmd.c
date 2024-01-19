#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lora_core.h"

#define TAG "LoRa AT cmd"
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
    LOG_I(TAG, "Disconnecting MAC: %s\n", cmd.data);
    StringToMac(cmd.data, mac);
    id = Compare_MAC(mac);
    if (id != 0xFF)
    {
        PCmd_Master_Request_Leave(id);
        LoRaDelSlaver(id);
        memset(&LoRaDevice.Slaver[id], 0, sizeof(LoRa_Node_t));
    }
}

void handle_connect(Command cmd)
{
    LOG_I(TAG, "Connecting MAC: %s\n", cmd.data);
    StringToMac(cmd.data, RegisterDevice.device.Mac);
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
    LOG_I(TAG, "Setting MAC: %s to new name: %s\n", mac, name);
    StringToMac(mac, mac8);
    id = Compare_MAC(mac8);
    PCmd_LoRa_SetSlaverName(name, id);
}

void handle_setSelfName(Command cmd)
{
    char BleBuffer[50];
    memset(BleBuffer, 0, sizeof(BleBuffer));
    PCmd_LoRa_SetSelfName(cmd.data);
    sprintf(BleBuffer, "AT+NAME=乐清%s\r\n", LoRaDevice.Self.name);
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    LOG_I(TAG, "Setting Self Name: %s\n", LoRaDevice.Self.name);
    delay_ms(500);
    sprintf(BleBuffer, "AT+REBOOT=1\r\n");
    UART1_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
}

void handle_bleconnect(Command cmd)
{
    char BleBuffer[50], macStr[24];
    delay_ms(100);
    ble_state.connect = 1;
    if (LoRaDevice.BandWidth == 2 && LoRaDevice.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__1__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    else if (LoRaDevice.BandWidth == 1 && LoRaDevice.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__2__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    else if (LoRaDevice.BandWidth == 0 && LoRaDevice.SpreadingFactor == 7)
        sprintf(BleBuffer, "PARA__%d__3__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    else if (LoRaDevice.BandWidth == 0 && LoRaDevice.SpreadingFactor == 8)
        sprintf(BleBuffer, "PARA__%d__4__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    else if (LoRaDevice.BandWidth == 0 && LoRaDevice.SpreadingFactor == 9)
        sprintf(BleBuffer, "PARA__%d__5__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    else
        sprintf(BleBuffer, "PARA__%d__Err__%s__%d__%d", LoRaDevice.NetMode, LoRaDevice.Self.name, (int)LoRaBackup.UART_Baud, LoRaDevice.channel);
    BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    if (LoRaDevice.Master.shortAddress < BoardCast && LoRaDevice.Master.shortAddress > 0) // master有效
    {
        memset(macStr, 0, sizeof(macStr));
        MacToString(LoRaDevice.Master.Mac, macStr);
        sprintf(BleBuffer, "MASTER__%s__%s__%d", LoRaDevice.Master.name, macStr, LoRaDevice.Master.RSSI);
        BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    }
    else
    {
        sprintf(BleBuffer, "MASTER__UnConnected__NULL__NULL");
        BLE_SendData((uint8_t *)BleBuffer, strlen(BleBuffer));
    }
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress != 0 && LoRaDevice.Slaver[i].shortAddress < BoardCast)
        {
            memset(macStr, 0, sizeof(macStr));
            MacToString(LoRaDevice.Slaver[i].Mac, macStr);
            sprintf(BleBuffer, "CON__%s__%s__%d", LoRaDevice.Slaver[i].name, macStr, LoRaDevice.Slaver[i].RSSI);
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
            // LOG_I(TAG, "AT Index:%d\r\n",result.index);
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
        LOG_I(TAG, "AT+SEND\r\n");
        break;
    default:
        LOG_I(TAG, "ERROR:No such device.\r\n");
        return;
    }
}

void handleSleep(int parameter)
{
    LOG_I(TAG, "AT+SLEEP\r\n");
}

void handleRst(int parameter)
{
    uint8_t temp_data[8];
    memset(temp_data, 0xFF, 8);
    LoRa_NetPara_Save(Type_Lora_net);
    LOG_I(TAG, "AT+RST\r\n");
    delay_ms(100);
    system_reset();
}

void handleChannel(int parameter)
{
    if (parameter < 0 || parameter > 100)
    {
        LOG_I(TAG, "ERROR:Invalid channel number.\r\n");
        return;
    }
    LOG_I(TAG, "AT+CHANNEL:%d\r\n", parameter);
    LoRaBackup.channel = parameter;
}

void handleAR(int parameter)
{
    if (parameter < 1 || parameter > 5)
    {
        LOG_I(TAG, "ERROR:Invalid bandwidth.\r\n");
        return;
    }
    switch (parameter)
    {
    case 1:
        LoRaBackup.BandWidth = 2;
        LoRaBackup.SpreadingFactor = 7;
        break;
    case 2:
        LoRaBackup.BandWidth = 1;
        LoRaBackup.SpreadingFactor = 7;
        break;
    case 3:
        LoRaBackup.BandWidth = 0;
        LoRaBackup.SpreadingFactor = 7;
        break;
    case 4:
        LoRaBackup.BandWidth = 0;
        LoRaBackup.SpreadingFactor = 8;
        break;
    case 5:
        LoRaBackup.BandWidth = 0;
        LoRaBackup.SpreadingFactor = 9;
        break;
    default:
        break;
    }
    LoRa_NetPara_Save(Type_Lora_net);
    LOG_I(TAG, "AT+AR:%d\r\n", parameter);
    LoraReInit();
    
}

void handleTxpower(int parameter)
{
    if (parameter < 0 || parameter > 21)
    {
        LOG_I(TAG, "ERROR:Invalid tx power.\r\n");
        return;
    }
    LOG_I(TAG, "AT+TXPOWER:%d\r\n", parameter);
    // Lora_Para_AT.Power = parameter;
}

void handlePanid(int parameter)
{
    // 0xFFFE为组播ID、0xFFFF为广播ID
    if (parameter < 1 || parameter > 0xFFFD)
    {
        LOG_I(TAG, "ERROR:Invalid panid number.\r\n");
        return;
    }
    LOG_I(TAG, "AT+PANID:%04X\r\n", parameter);
    LoRaBackup.PanID = parameter;
}

void handleSaddr(int parameter)
{
    if (parameter < 1 || parameter > 0xFFFD)
    {
        LOG_I(TAG, "ERROR:Invalid SADDR number.\r\n");
        return;
    }
    LOG_I(TAG, "AT+SADDR:%04X\r\n", parameter);
    LoRaBackup.SAddr = parameter;
}

void handleNetopen(int parameter)
{
    LOG_I(TAG, "AT+NETOPEN:%d\r\n", parameter % 256);
    if (parameter == 1)
        LoRaDevice.NetMode = 1;
    else if (parameter == 2)
        LoRaDevice.NetMode = 2;
    else
        LoRaDevice.NetMode = 0;
    LoRaDevice.NetMode = LoRaDevice.NetMode;
    LoRa_NetPara_Save(Type_Lora_net);
    Reset_LoRa();
}

void handleNetclose(int parameter)
{
    LOG_I(TAG, "AT+NETCLOSE\r\n");
    LoRaDevice.NetMode = 0;
    LoRa_NetPara_Save(Type_Lora_net);
    Reset_LoRa();
}

void handleLeave(int parameter)
{
    LOG_I(TAG, "AT+LEAVE\r\n");
    // 处理逻辑...
    PCmd_Slaver_Request_Leave();
}

void handleDelet(int parameter)
{
    if (parameter < 0 || parameter > MAX_CHILDREN)
    {
        LOG_I(TAG, "Invalid delet number.\r\n");
        return;
    }
    LOG_I(TAG, "AT+DELET:%d\r\n", parameter);
    PCmd_Master_Request_Leave(parameter);
    LoRaDelSlaver(parameter);
    memset(&LoRaDevice.Slaver[parameter], 0, sizeof(LoRa_Node_t));
}

void handlePrint(int parameter)
{

}

void handleBaud(int parameter)
{
    if (parameter == 9600)
    {
        LoRaBackup.UART_Baud = 9600;
    }
    else if (parameter == 19200)
    {
        LoRaBackup.UART_Baud = 19200;
    }
    else if (parameter == 38400)
    {
        LoRaBackup.UART_Baud = 38400;
    }
    else if (parameter == 57600)
    {
        LoRaBackup.UART_Baud = 57600;
    }
    else if (parameter == 115200)
    {
        LoRaBackup.UART_Baud = 115200;
    }
    else
    {
        LOG_I(TAG, "ERROR:Invalid baud rate.\r\n");
        return;
    }
    LOG_I(TAG, "AT+BAUD:%d\r\n", parameter);
    LoRa_NetPara_Save(Type_Lora_net);
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
        LOG_I(TAG, "index:%d \r\n", parsed_command.index);
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