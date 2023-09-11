#!/usr/bin/env python3

import os

#list of symbols to generate and their font file
symbols_array = [
[0xe568,'HEAT_BED','fa-solid-900.ttf'],
[0xf2c9,'EXTRUDER','fa-solid-900.ttf'],
[0xf0ca,'LIST','fa-solid-900.ttf'],
[0xf715,'SLASH','fa-solid-900.ttf'],
[0xf012,'STATION_MODE','fa-solid-900.ttf'],
[0xf519,'ACCESS_POINT','fa-solid-900.ttf'],
[0xf00c,'OK','fa-solid-900.ttf'],
[0xe596,'PROBE_CHECK','fa-solid-900.ttf'],
[0xf00d,'CLOSE','fa-solid-900.ttf'],
[0xf011,'POWER','fa-solid-900.ttf'],
[0xf028,'VOLUME_HIGH','fa-solid-900.ttf'],
[0xf027,'VOLUME_LOW','fa-solid-900.ttf'],
[0xf6a9,'VOLUME_OFF','fa-solid-900.ttf'],
[0xf013,'SETTINGS','fa-solid-900.ttf'],
[0xf2d1,'NO_HEAT_BED','fa-solid-900.ttf'],
[0xe040,'HEAT_EXTRUDER','fa-solid-900.ttf'],
[0xf2ed,'TRASH','fa-solid-900.ttf'],
[0xe3af,'HOME','fa-solid-900.ttf'],
[0xf019,'DOWNLOAD','fa-solid-900.ttf'],
[0xf021,'REFRESH','fa-solid-900.ttf'],
[0xf304,'EDIT','fa-solid-900.ttf'],
[0xf048,'PREVIOUS','fa-solid-900.ttf'],
[0xf051,'NEXT','fa-solid-900.ttf'],
[0xf04b,'PLAY','fa-solid-900.ttf'],
[0xf04c,'PAUSE','fa-solid-900.ttf'],
[0xf0c7,'SAVE','fa-solid-900.ttf'],
[0xf0e0,'MESSAGE','fa-solid-900.ttf'],
[0xf0e7,'LASER','fa-solid-900.ttf'],
[0xf76f,'VACCUM','fa-solid-900.ttf'],
[0xf1f6,'DISABLE_ALERT','fa-solid-900.ttf'],
[0xf023,'LOCK','fa-solid-900.ttf'],
[0xf2dc,'COOLANT','fa-solid-900.ttf'],
[0xf04d,'STOP','fa-solid-900.ttf'],
[0xf1eb,'WIFI','fa-solid-900.ttf'],
[0xf071,'WARNING','fa-solid-900.ttf'],
[0xf07b,'FOLDER','fa-solid-900.ttf'],
[0xf15b,'FILE','fa-solid-900.ttf'],
[0xf11c,'KEYBOARD','fa-solid-900.ttf'],
[0xf55a,'BACKSPACE','fa-solid-900.ttf'],
[0xf7c2,'SD_CARD','fa-solid-900.ttf'],
[0xf0b2,'JOG','fa-solid-900.ttf'],
[0xf077,'UP','fa-solid-900.ttf'],
[0xf078,'DOWN','fa-solid-900.ttf'],
[0xf053,'LEFT','fa-solid-900.ttf'],
[0xf054,'RIGHT','fa-solid-900.ttf'],
[0xf120,'COMMAND','fa-solid-900.ttf'],
[0xf624,'GAUGE','fa-solid-900.ttf'],
[0xf1ab,'LANGUAGE','fa-solid-900.ttf'],
[0xf863,'FAN','fa-solid-900.ttf'],
[0xf48b,'SPEED','fa-solid-900.ttf'],
[0xf72b,'WIZARD','fa-solid-900.ttf'],
[0xf185,'LIGHT','fa-solid-900.ttf'],
[0xf5c1,'ENGINE','fa-solid-900.ttf'],
[0xf5fd,'LAYERS','fa-solid-900.ttf'],
[0xe4b8,'LEVELING','fa-solid-900.ttf'],
[0xf4db,'FILAMENT','fa-solid-900.ttf'],
[0xe4bd,'CENTER','fa-solid-900.ttf'],
[0xf002,'SEARCH','fa-solid-900.ttf'],
[0xf4d7,'FILAMENT_SENSOR','fa-solid-900.ttf'],
[0xf2cc,'MIST','fa-solid-900.ttf'],
[0xf13e,'UNLOCK','fa-solid-900.ttf'],
[0xf192,'LASER_2','fa-solid-900.ttf'],
[0xe4c3,'MILLING','fa-solid-900.ttf'],
[0xf3e5,'NEW_LINE','fa-solid-900.ttf'],
[0xf293,'BLUETOOTH','fa-brands-400.ttf'],
[0xf287,'USB','fa-brands-400.ttf'],
[0xf0a1,'MORE_INFO','fa-solid-900.ttf'],
[0xf055,'PLUS','fa-solid-900.ttf'],
[0xf056,'MINUS','fa-solid-900.ttf'],
[0xf256,'MANUAL','fa-solid-900.ttf'],
[0xf544,'AUTOMATIC','fa-solid-900.ttf'],
]

#list of font sizes to generate
fonts_size_list = ["8","10","12","14","16","18","20","22","24","26","28","30","32","34","36","38","40","42","44","46","48"]

symboles=""

fonts_files_list=[]
#generate font
def generateFont(sizefont):
	mainfont = "Montserrat-Medium.ttf"
	fontname = "lv_font_montserrat_"
	rangemain= "0x20-0x7F,0xB0"
	#add French characters accute and grave accent and cedilla for small case, no accent for capital case
	rangemain+=",0xE0,0xE7,0xE8,0xE9,0xEA,0xF4"
    #add Chinese characters
	chinese_traditionnal_font = "NotoSansTC-Regular.ttf"
	rangechinese="0x7e41,0x9ad4,0x4e2d,0x6587,0x66f4,0x65b0,0x5927,0x5c0f,0x5c4f,0x5e55,0x62f1,0x5f62,0x983b,0x7387,0x9583,0x5b58,0x7a7a,0x9592,0x5806,0x7248,0x672c,0x7a0b,0x5e8f,0x958b,0x95dc,0x9589,0x6beb,0x7c73,0x4ef6,0x7cfb,0x7d71,0x8acb,0x78ba,0x8a8d,0x60a8,0x60f3,0x505c,0x6b62,0x7576,0x524d,0x6253,0x5370,0x55ce,0xff1f,0x932f,0x8aa4,0x61c9,0x7528,0x6a21,0x5f0f,0x5931,0x6557,0xff01,0x8a2d,0x7f6e,0x4e3b,0x6a5f,0x540d,0x64f4,0x5c55,0x7a0d,0x7b49,0x8f38,0x51fa,0x7aef,0x53e3,0x4e32,0x884c,0x6ce2,0x7279,0x6b63,0x5728,0x9023,0x63a5,0x7121,0x7dda,0x7db2,0x7d61,0x672a,0x70ba,0x5165,0x9ede,0x4e1f,0x52d5,0x985e,0x578b,0x8179,0x808c,0x7d55,0x5c0d,0x76f8,0x8f2a,0x8a62,0x5df2,0x555f,0x7981,0x96fb,0x98a8,0x6247,0x63a7,0x5236,0x4fe1,0x606f,0x81ea,0x8abf,0x5e73,0x7bc0,0x6fc0,0x6d3b,0x4f4d,0x7684,0x5404,0x500b,0x65cb,0x9215,0xff0c,0x76f4,0x5230,0x4e00,0x5f35,0x7d19,0x6070,0x597d,0x5674,0x5634,0x548c,0x677f,0x4e4b,0x9593,0x6ed1,0x3002,0x6bcf,0x91cd,0x8907,0x64cd,0x4f5c,0x4e0d,0x9700,0x8981,0x6821,0x5c07,0x81f3,0x6c34,0x7136,0x5f8c,0x55ae,0x64ca,0x201c,0x4e0b,0x6b65,0x201d,0x4e5f,0x53ef,0x4ee5,0x624b,0x9078,0x64c7,0x5e8a,0x5bec,0x5c64,0x6df1,0x5ea6,0x63a2,0x6e2c,0xff1a,0x59cb,0x53cd,0x8f49,0x8ef8,0x8a9e,0x8a00"
	source = "lv_font_conv --no-compress --no-prefilter --bpp 4 --size "+sizefont+" --font " + mainfont + " -r "+rangemain +" --font " + chinese_traditionnal_font + " -r "+rangechinese + " --font FreeSerifBold.ttf -r 0x2022" +symboles+" --format lvgl -o fonts/"+fontname+sizefont+".c --force-fast-kern-format"
	print("Generating "+sizefont+" px, "+fontname+sizefont+".c" )
	print(source)
	os.system(source)

#list all font files
for item in symbols_array:
	if item[2] not in fonts_files_list:
		fonts_files_list.append(item[2])
#generate symbols list for each font file
for font_file_name in fonts_files_list:
	symboles+=" --font "+font_file_name+" -r "
	for item in symbols_array:
		if item[2] == font_file_name:
			symboles=symboles+str(item[0])+","
	symboles = symboles[:-1]
#generate fonts for each size
for font_size in fonts_size_list:
	generateFont(font_size)


