#ifndef _780EG__H__
#define _780EG__H__

#define REV_OK   0 // 接收完成标志
#define REV_WAIT 1 // 接收未完成标志

// AT指令
#define AT_EMQX_IP      "AT+MIPSTART=\"8.134.252.177\",\"1883\"\r\n" // EMQX地址
#define AT_EMQX_Client  "AT+MCONFIG=\"19\",\"lbd\",\"lbd123\"\r\n"   // 客户端ID及地址
#define AT_EMQX_CONNECT "AT+MCONNECT=1,2000\r\n"                       // 客户端向服务器请求会话连接,具体参数见AT手册
#define AT_EMQX_PUB     "AT+MPUB=\"test\",1,0,\"666\"\r\n"           // 客户端向服务器请求会话连接,具体参数见AT手册

enum DataType // 枚举数据类型
{
    Int,
    Float
};

struct Data{
    char msg[30];//消息名
    void *value; // 参数值
    enum DataType type;
};

void Air780EG_Clear(void);                                                                   // 清缓存
_Bool Air780EG_WaitRecive(void);                                                             // 等待接收
_Bool Air780EG_Sendcmd(char *cmd, char *ret, char *respond);                                 // 发送命令
_Bool Air780EG_SendcmdReceive2key(char *cmd, char *ret1, char *ret2);                        // 发送命令，并检查发送回来的两个关键字符串
void Air780EG_Init();                                                                        // 初始化
unsigned char save_jsonData(char *json_output, char msg[], void *value, enum DataType Type); // 将数据封装成json格式
unsigned char save_therejsonData(char *json_output, float *SPO2_value, int *heart_value, int *temp_value);
void Air780EG_Sendmqttdata(enum DataType Type, char topic[], int qos, int retain, void *data, char msg[]); // 发送数据到EMQX
void Air780EG_Sendtheremqttdata(char topic[], float *SPO2_data, int *heart_data, int *temp_data);
void Air780EG_GNSSInit();
_Bool Air780EG_sendGNSSdata();

void Air780EG_testSendmqttdata(char topic[], int qos, int retain, struct Data data_array[], int count, char *respond);//测试

#endif