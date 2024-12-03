<span align="left"><img src="https://github.com/luc-github/ESP3D-TFT/blob/main/resources/logo/ESP3D.png" width="200px"/></span><span align="left">ESP3D-TFT Firmware for ESP32 based TFT  </span>    
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-3-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[<img src="https://img.shields.io/liberapay/patrons/ESP3D.svg?logo=liberapay">](https://liberapay.com/ESP3D)

[![Development Version](https://img.shields.io/badge/Dev-v1.0-yellow?style=plastic) ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D-TFT/main?style=plastic)](https://github.com/luc-github/ESP3D-TFT/tree/main) [![github-ci](https://github.com/luc-github/ESP3D-TFT/workflows/build-ci/badge.svg)](https://github.com/luc-github/ESP3D-TFT/actions/workflows/build-ci.yml) [![Development  Version](https://img.shields.io/badge/Dev-v3.0-yellow?style=plastic&label=WebUI&logo=Preact)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0) [![IDF 5.1.5](https://img.shields.io/badge/IDF-v5.1.5-blue?style=plastic&label=IDF&logo=espressif)](https://github.com/espressif/esp-idf)
![ESP3D-TFT](https://img.shields.io/badge/dynamic/json?label=ESP3D-TFT&query=$.version&url=https://raw.githubusercontent.com/luc-github/ESP3D-TFT/refs/heads/main/info.json)

> [!WARNING]
>### Disclaimer
> The software is provided 'as is,' without any warranty of any kind, expressed or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.
>It is essential that you carefully read and understand this disclaimer before using this software and its components. If you do not agree with any part of this disclaimer, please refrain from using the software.  

## Features
A Serial TFT with all WiFi feature to control 3D Printer or CNC and [webui](https://githubcom/luc-github/ESP3D-WEBUI)

Unlike ESP3D some features will be fixed:
* Flash FS = littleFS
* ESP32 / ESP32-S3
* WebDav on Both local FS and SD

> [!WARNING]
>### Disclaimer
> The software is provided 'as is,' without any warranty of any kind, expressed or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.
>It is essential that you carefully read and understand this disclaimer before using this software and its components. If you do not agree with any part of this disclaimer, please refrain from using the software.  

## UI

lvgl library 8.3.8

## Framework

Espressif IDF 5.1.5 

## Reference boards

* ESP32 based + SDReader + 2.8'resistive screen (240x320) [model](https://www.aliexpress.com/item/3256804315935867.html)
* ESP32 + SDReader + 3.5' Resistive screen (480x320) [Rotrics TFT](https://rotrics.com/products/3-5-inch-touchscreen)
* ESP32-S3 based + SDReader + PSRAM + 4.3' capacitive screen  (800x600) [model](https://www.aliexpress.com/item/1005003814428825.html)
* ESP32-S3 based + SDReader + PSRAM + 3.5' capacitive screen (320x480) [model](https://www.aliexpress.com/item/1005004309826174.html)

* More on https://esp3d.io/esp3d-tft/v1.x/hardware/index.html

## installation
Please follow the instructions from https://esp3d.io/esp3d-tft/v1.x/installation/index.html


## Todo

Everything, code from scratch or almost
* Design UI (Flow + icon)
* Code UI / navigation 
* Add WiFi features
* Add Streaming Features 
* TBD... 

## Chat

Please use discord : [![discord](https://img.shields.io/discord/752822148795596940?color=blue&label=discord&logo=discord)](https://discord.gg/Z4ujTwE)

## :question:Any question ?   
Check [Wiki](https://github.com/luc-github/ESP3D/wiki/Install-Instructions) or Join the chat at [![Discord server](https://img.shields.io/discord/752822148795596940?color=blue&label=discord&logo=discord)](https://discord.gg/Z4ujTwE)   

## :exclamation:Any issue/feedback ?    
Check [http://esp3dio](http://esp3dio) for more information
If you still have issue: [submit ticket](https://github.com/luc-github/ESP3D-TFT/issues)    
If it is not an issue join some discussion [here](https://github.com/luc-github/ESP3D-TFT/discussions)

## Sponsors 
[<img width="200px" src="https://raw.githubusercontent.com/luc-github/ESP3D-WEBUI/2.1/images/sponsors-supporters/MKS/mksmakerbase.jpg" title="MKS Makerbase">](https://github.com/makerbase-mks)&nbsp;&nbsp;

## Supporters

## Become a sponsor or a supporter
 * A sponsor is a recurent donator    
   As my sponsorship is not displayed by github your logo / avatar will be added to the readme page with eventually with a link to your site.    
 * A supporter is per time donator 
   To thank you for your support, your logo / avatar will be added to the readme page with eventually with a link to your site.  

 Every support is welcome, indeed helping users / developing new features need time and devices, donations contribute a lot to make things happen, thank you.

* liberapay <a href="https://liberapay.com/ESP3D/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a> 
* Paypal [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FQL59C749A78L)
* ko-fi [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G0C0QT7)   

## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/3DSmitty"><img src="https://avatars.githubusercontent.com/u/51137582?v=4?s=100" width="100px;" alt="3DSmitty"/><br /><sub><b>3DSmitty</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D-TFT/commits?author=3DSmitty" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/serisman"><img src="https://avatars.githubusercontent.com/u/670207?v=4?s=100" width="100px;" alt="serisman"/><br /><sub><b>serisman</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D-TFT/commits?author=serisman" title="Code">💻</a> <a href="#ideas-serisman" title="Ideas, Planning, & Feedback">🤔</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/jamespearson04"><img src="https://avatars.githubusercontent.com/u/26628667?v=4?s=100" width="100px;" alt="James Pearson"/><br /><sub><b>James Pearson</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D-TFT/commits?author=jamespearson04" title="Code">💻</a> <a href="#ideas-jamespearson04" title="Ideas, Planning, & Feedback">🤔</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
