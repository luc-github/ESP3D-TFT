#!/usr/bin/env python3

import os
# Path: scripts/language_packs/build_template.py
##Translation pack for french language
#[translations]
#l_0=Français
#
#[info]
#creator=luc@tech-hunter.com
#creation_date=2023-05-16
#maintainer==luc@tech-hunter.com
#last_update=2023-05-16
#target_language=French
#target_version=3.0

output_file = "template.lng"
input_file = "../../main/target/3dprinter/marlin/esp3d_translations_list.h"
input_translation_file = "../../main/target/3dprinter/marlin/esp3d_translations_init.cpp"

#parse esp3d_translations_list.h for each entry and add it to the array in same order
#parse esp3d_translations_init.cpp for each entry and add it to the array with english translation
translation_label_array = []

header_file = "#Translation pack for <language> language\n[translations]\n"
footer_file = "\n[info]\ncreator=<creator>\ncreation_date=<creation_date>\nmaintainer=<maintainer>\nlast_update=<last_update>\ntarget_language=<target_language>\ntarget_version=<target_version>\n"
#Open file for writing and overwrite current
f = open(output_file, "w")
f.write(header_file)
#read label file to get proper indexes
fi_handle = open(input_file, "r")
Lines = fi_handle.readlines()
index=0
startEntry = False
for line in Lines:
    if line.strip() == "enum class ESP3DLabel : uint16_t {":
        startEntry = True
        continue
    if startEntry and line.strip() == "};":
        break
    if startEntry:
        entry = line.strip()
        p = entry.find("//")
        if p != -1:
            entry = entry[:p].strip()
        p = entry.find("=")
        if p != -1:
            entry = entry[:p].strip()
        entry = entry.strip().strip(",")
        if entry != "unknown_index":
            translation_label_array.append( [entry,"l_"+str(index),""])
            print("Line{}: {}".format(index, entry))
            index += 1
fi_handle.close()

#read translation file to get proper translations
fi_handle = open(input_translation_file, "r")
Lines = fi_handle.readlines()
for line in Lines:
     p = line.find("{ESP3DLabel::")
     if p != -1:
        p2 = line.find("}")
        if p2 != -1:
            entry = line[p+13:p2]
            data = entry.split(",")
            for trans in translation_label_array:
                if data[0].strip()==trans[0]:
                    trans[2]=data[1].strip()
fi_handle.close()
#save all to file
for line in translation_label_array:
    f.write(line[1]+"="+line[2]+"\n")
#Write footer
f.write(footer_file)
#Close file
f.close()