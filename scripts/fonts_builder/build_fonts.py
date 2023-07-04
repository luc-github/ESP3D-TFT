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
[0xf05b,'CENTER','fa-solid-900.ttf'],
[0xf5fd,'LAYERS','fa-solid-900.ttf'],
[0xe4b8,'LEVELING','fa-solid-900.ttf'],
[0xf4db,'FILAMENT','fa-solid-900.ttf'],
[0xe4bd,'CENTER2','fa-solid-900.ttf'],
[0xf002,'SEARCH','fa-solid-900.ttf'],
[0xf4d7,'FILAMENT_SENSOR','fa-solid-900.ttf'],
[0xf2cc,'MIST','fa-solid-900.ttf'],
[0xf13e,'UNLOCK','fa-solid-900.ttf'],
[0xf192,'LASER_2','fa-solid-900.ttf'],
[0xe4c3,'MILLING','fa-solid-900.ttf'],
[0xf3e5,'NEW_LINE','fa-solid-900.ttf'],
[0xf293,'BLUETOOTH','fa-brands-400.ttf'],
[0xf287,'USB','fa-brands-400.ttf'],
[0xf0a1,'MORE_INFO','fa-solid-900.ttf']
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
	source = "lv_font_conv --no-compress --no-prefilter --bpp 4 --size "+sizefont+" --font " + mainfont + " -r "+rangemain + " --font FreeSerifBold.ttf -r 0x2022" +symboles+" --format lvgl -o fonts/"+fontname+sizefont+".c --force-fast-kern-format"
	print("Generating "+sizefont+" px, "+fontname+sizefont+".c" )
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


