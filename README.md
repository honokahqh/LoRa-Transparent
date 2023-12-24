# 乐清蓝牙模组

# 中继设置相关
# 手机蓝牙发送AT+ADDENDPOINT__<NAME>__
# 中继点广播命令 SEARCH_ENDPOINT  中继名__终端节点名称
# 终端节点回复命令  SEARCH_ENDPOINT_ACK 中继名__终端节点名称 
# 终端节点保存中继名 短地址,此后发送均指定SADDR,命令改为NEED_RELAY
# 中继节点收到ACK,保存终端节点短地址 名称,此后收到终端节点包，则透传命令广播,若收到任意其他地址包,