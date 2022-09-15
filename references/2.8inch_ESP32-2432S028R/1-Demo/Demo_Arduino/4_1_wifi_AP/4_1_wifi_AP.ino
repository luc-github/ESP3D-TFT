#include <Arduino.h>
#include "WiFi.h"
void setup()
{
  Serial.begin(115200);
  WiFi.softAP("ESP_AP", "12345678");
}
void loop()
{
  Serial.print("主机名:");
  Serial.println(WiFi.softAPgetHostname());
  Serial.print("主机IP:");
  Serial.println(WiFi.softAPIP());
  Serial.print("主机IPV6:");
  Serial.println(WiFi.softAPIPv6());
  Serial.print("主机SSID:");
  Serial.println(WiFi.SSID());
  Serial.print("主机广播IP:");
  Serial.println(WiFi.softAPBroadcastIP());
  Serial.print("主机mac地址:");
  Serial.println(WiFi.softAPmacAddress());
  Serial.print("主机连接个数:");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.print("主机网络ID:");
  Serial.println(WiFi.softAPNetworkID());
  Serial.print("主机状态:");
  Serial.println(WiFi.status());
  delay(1000);
}
