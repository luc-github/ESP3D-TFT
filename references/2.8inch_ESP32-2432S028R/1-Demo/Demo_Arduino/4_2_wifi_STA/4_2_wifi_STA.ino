#include <Arduino.h>
#include "WiFi.h"
void setup()
{
  Serial.begin(115200);

  WiFi.begin("iPhone", "12345678");
  WiFi.setAutoReconnect(true);
}
void loop()
{
  Serial.print("是否连接:");
  Serial.println(WiFi.isConnected());
  Serial.print("本地IP:");
  Serial.println(WiFi.localIP());
  Serial.print("本地IPv6:");
  Serial.println(WiFi.localIPv6());
  Serial.print("mac地址:");
  Serial.println(WiFi.macAddress());
  Serial.print("网络ID:");
  Serial.println(WiFi.networkID());
  Serial.print("休息:");
  Serial.println(WiFi.getSleep());
  Serial.print("获取状态码:");
  Serial.println(WiFi.getStatusBits());
  Serial.print("getTxPower:");
  Serial.println(WiFi.getTxPower());
  Serial.print("是否自动连接:");
  Serial.println(WiFi.getAutoConnect());
  Serial.print("是否自动重连:");
  Serial.println(WiFi.getAutoReconnect());
  Serial.print("获取模式:");
  Serial.println(WiFi.getMode());
  Serial.print("获取主机名:");
  Serial.println(WiFi.getHostname());
  Serial.print("获取网关IP:");
  Serial.println(WiFi.gatewayIP());
  Serial.print("dnsIP:");
  Serial.println(WiFi.dnsIP());
  Serial.print("状态:");
  Serial.println(WiFi.status());
  delay(1000);
}
