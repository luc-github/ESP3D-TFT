#include <WiFi.h>

const char *ssid = "CMCC-404";
const char *password = "12345678";

WiFiServer server; //声明服务器对象

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());

    server.begin(10000); //服务器启动监听端口号22333
}

void loop()
{
    WiFiClient client = server.available(); //尝试建立客户对象
    if (client) //如果当前客户可用
    {
        Serial.println("[Client connected]");
        String readBuff;
        while (client.connected()) //如果客户端处于连接状态
        {
            if (client.available()) //如果有可读数据
            {
                char c = client.read(); //读取一个字节
                                        //也可以用readLine()等其他方法
                readBuff += c;
               if(c == '\r') //接收到回车符
                {
                    client.print("Received: " + readBuff); //向客户端发送
                    Serial.println("Received: " + readBuff); //从串口打印
                    readBuff = "";

                }
            }
        }
        client.stop(); //结束当前连接:
        Serial.println("[Client disconnected]");
    }
}
