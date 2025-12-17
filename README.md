<span align="left"><img src="https://github.com/luc-github/ESP3D-TFT/blob/main/resources/logo/ESP3D.png" width="200px"/></span><span align="left">ESP3D-TFT Firmware for ESP32 based TFT - State: Alpha (do not rush on it yet)</span>    
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-4-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

![ESP3D-TFT](https://img.shields.io/badge/dynamic/json?label=Development&query=$.devt&color=green&style=plastic&url=https://raw.githubusercontent.com/luc-github/ESP3D-TFT/refs/heads/main/info.json)
![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D-TFT/main?style=plastic)
[![github-ci](https://github.com/luc-github/ESP3D-TFT/workflows/build-ci/badge.svg)](https://github.com/luc-github/ESP3D-TFT/actions/workflows/build-ci.yml) 
[![Development  Version](https://img.shields.io/badge/Dev-v3.0-yellow?style=plastic&label=WebUI&logo=Preact)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0) 
[![IDF 5.1.5](https://img.shields.io/badge/IDF-v5.1.5-blue?style=plastic&label=IDF&logo=espressif)](https://github.com/espressif/esp-idf)


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

## Sponsors 
<div align="center">
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/diamond-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/diamond-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/platinum-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/platinum-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/gold-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/gold-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d-tft/silver-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d-tft/silver-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   Support ESP3D Development - <a href="https://esp3d.io/sponsoring" target="_blank" rel="noopener noreferrer">Become a Sponsor</a>
</div>

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
      <td align="center" valign="top" width="14.28%"><a href="https://discord.gg/yNwksQvZmQ"><img src="https://avatars.githubusercontent.com/u/12979070?v=4?s=100" width="100px;" alt="makerbase"/><br /><sub><b>makerbase</b></sub></a><br /><a href="#financial-makerbase-mks" title="Financial">💵</a> <a href="#platform-makerbase-mks" title="Packaging/porting to new platform">📦</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
