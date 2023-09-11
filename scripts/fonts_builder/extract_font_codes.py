#!/usr/bin/env python3

import os

input_file = "../../languagePacks/ui_zh_tw.lng"
f = open(input_file, "r", encoding='utf-8')
lines = f.readlines()
started = False
symbols_array = []
for line in lines:
    if line.strip() == "[translations]":
        started = True
        continue
    if line.strip() == "[info]":
        started = False
        continue
    if started:
        if line.strip() == "":
            continue
        else:
            line = line.strip()
            line = line.split("=", 1)
            for char in line[1]:
                if char not in symbols_array and ord(char) > 0xB0:
                    print(char)
                    symbols_array.append(char)
            #print(line[1])

f.close()
line = ""
for char in symbols_array:
    line += hex(ord(char)) + ","
print(line)