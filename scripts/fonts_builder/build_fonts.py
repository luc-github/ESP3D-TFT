#!/usr/bin/env python3

import os

def generateFont(sizefont):
	mainfont = "Montserrat-Medium.ttf"
	fontname = "lv_font_montserrat_"
	rangemain= "0x20-0x7F,0xB0,0x2022"
	symbolsfont = "fa-solid-900.ttf"
	rangesymbols= "58728,62153,61642,63231,61458,62745,61452,58774,61453,61457,61480,61479,63145,61459,61918,63449,62189,58287,61465,61473,62212,61512,61521,61515,61516,61639,61664,61671,63343,61942,61475,62172,61517,61931,61553,61563,61787,61724,62810,63426,61618,61559,61560,61523,61524,61728,63012,61867,63587,62603,63275,61829,61573,62973,58552,62683,58557,61442,62679,62156,61758,61842,58563"
	symbolsfont2 = "fa-brands-400.ttf"
	rangesymbols2= "62099,62087"
	source = "lv_font_conv --no-compress --no-prefilter --bpp 4 --size "+sizefont+" --font " + mainfont + " -r "+rangemain+" --font "+symbolsfont+" -r "+rangesymbols+" --font "+symbolsfont2+" -r "+rangesymbols2+" --format lvgl -o fonts/"+fontname+sizefont+".c --force-fast-kern-format"
	print("Generating "+sizefont+" px, "+fontname+sizefont+".c" )
	os.system(source)

generateFont("8")
generateFont("10")
generateFont("12")
generateFont("14")
generateFont("16")
generateFont("18")
generateFont("20")
generateFont("22")
generateFont("24")
generateFont("26")
generateFont("28")
generateFont("30")
generateFont("32")
generateFont("34")
generateFont("36")
generateFont("38")
generateFont("40")
generateFont("42")
generateFont("44")
generateFont("46")
generateFont("48")

