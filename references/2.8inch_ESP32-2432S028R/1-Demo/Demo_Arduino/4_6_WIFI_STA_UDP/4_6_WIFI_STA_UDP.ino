#include <WiFi.h>
#include <AsyncUDP.h> //引用以使用异步UDP

const char *ssid = "CMCC-404";
const char *password = "12345678";

AsyncUDP udp;                     //创建UDP对象
unsigned int localUdpPort = 10000; //本地端口号

unsigned int broadcastPort = localUdpPort;
const char *broadcastData = "broadcast data";
// const uint8_t broadcastData[] = {"broadcast data"};

void onPacketCallBack(AsyncUDPPacket packet)
{
  Serial.print("UDP数据包来源类型: ");
  Serial.println(packet.isBroadcast() ? "广播数据" : (packet.isMulticast() ? "组播" : "单播"));
  Serial.print("远端地址及端口号: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.println(packet.remotePort());
  Serial.print("目标地址及端口号: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.println(packet.localPort());
  Serial.print("数据长度: ");
  Serial.println(packet.length());
  Serial.print("数据内容: ");
  Serial.write(packet.data(), packet.length());
  Serial.println();

  packet.print("reply data");
  broadcastPort = packet.remotePort();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected())
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());

  while (!udp.listen(localUdpPort)) //等待udp监听设置成功
  {
  }
  udp.onPacket(onPacketCallBack); //注册收到数据包事件
}

void loop()
{
  delay(5000);

  udp.broadcastTo(broadcastData, broadcastPort); //可以使用该方法广播信息

  // IPAddress broadcastAddr((~(uint32_t)WiFi.subnetMask())|((uint32_t)WiFi.localIP())); //计算广播地址
  // udp.writeTo(broadcastData, sizeof(broadcastData), broadcastAddr, localUdpPort); //广播数据
}
