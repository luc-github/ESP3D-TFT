# Fonts builder for ESP3D-TFT   
1 - Install font converter (nodejs is required)    
https://github.com/lvgl/lv_font_conv
`npm i lv_font_conv -g`

2 - Edit build fonts script according your needs
  build_fonts.py

3 - Run font packager for all resolutions  
`build_fonts.py`

4 - Generate a new lv_symbol_def.h   
Based on https://docs.lvgl.io/master/overview/font.html#add-new-symbols
`build_defs.py`

5 - copy content of `fonts` directory to `components\lvgl\src\font`
