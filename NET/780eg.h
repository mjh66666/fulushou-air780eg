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
void Air780EG_Init();                                                                        // 初始化
void Air780EG_Sendonemqttdata(enum DataType Type, char topic[], int qos, int retain, void *data, char msg[]); // 发送数据到EMQX
void Air780EG_GNSSInit();
_Bool Air780EG_sendGNSSdata(float *longitude, float *latitude);
void Air780EG_Sendmqttdata(char topic[], int qos, int retain, struct Data data_array[], int count, char *respond); // 测试

#endif