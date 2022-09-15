#include <Arduino.h>
#include "WiFi.h"

void mySmartWifiConfig()
{
  WiFi.mode(WIFI_MODE_STA);
  Serial.println("开启智能配网:");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    delay(500);
    if (WiFi.smartConfigDone())
    {
      Serial.println("配网成功");
      Serial.printf("SSID:%s", WiFi.SSID().c_str());
      Serial.printf("PSW:%s", WiFi.psk().c_str());
      break;
    }
  }
}

bool autoConfig()
{
  WiFi.disconnect(true,true);
  WiFi.begin();
  for (size_t i = 0; i < 20; i++)
  {
    int wifiStatus = WiFi.status();
    if (wifiStatus == WL_CONNECTED)
    {
      Serial.println("自动连接成功!");
      return 1;
    }
    else
    {
      delay(1000);
      Serial.println("等待自动配网中...");
    }
  }
  Serial.println("无法自动配网!");
  return 0;
}

void setup()
{
  Serial.begin(115200);
  delay(5000);
  if (!autoConfig())
  {
    mySmartWifiConfig();
  }
}
void loop()
{
  if (WiFi.isConnected())
  {
    Serial.println("网络已连接");
    delay(1000);
  }
}
