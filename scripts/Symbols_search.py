import os
import argparse

def search_defines(defines, directory, show_only_not_found):
    for define in defines:
        found = False
        for root, dirs, files in os.walk(directory):
            for file in files:
                if file.endswith(('.c', '.h', '.cpp', '.hpp')):
                    file_path = os.path.join(root, file)
                    try:
                        with open(file_path, 'r', encoding='utf-8') as f:
                            content = f.read()
                            if define in content:
                                if not show_only_not_found:
                                    if not found:
                                        print(f"Searching '{define}' :")
                                    print(f"  - Used in : {file_path}")
                                found = True
                    except UnicodeDecodeError:
                        ConnectionRefusedError
        if not found:
            if (not show_only_not_found):
                print(f"Searching '{define}' : not found in any file")
            else:
                print(f"'{define}'")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Search in sources files for symboles')
    parser.add_argument('-n', '--not-found', action='store_true', help='Display only not found symboles')
    args = parser.parse_args()

    # Array definition of the defines to search
    defines = ['LV_SYMBOL_HEAT_BED','LV_SYMBOL_EXTRUDER','LV_SYMBOL_LIST','LV_SYMBOL_SLASH','LV_SYMBOL_STATION_MODE','LV_SYMBOL_ACCESS_POINT','LV_SYMBOL_OK','LV_SYMBOL_PROBE_CHECK','LV_SYMBOL_CLOSE','LV_SYMBOL_POWER','LV_SYMBOL_VOLUME_HIGH','LV_SYMBOL_VOLUME_LOW','LV_SYMBOL_VOLUME_OFF','LV_SYMBOL_SETTINGS','LV_SYMBOL_NO_HEAT_BED','LV_SYMBOL_HEAT_EXTRUDER','LV_SYMBOL_TRASH','LV_SYMBOL_HOME','LV_SYMBOL_DOWNLOAD','LV_SYMBOL_REFRESH','LV_SYMBOL_EDIT','LV_SYMBOL_PREVIOUS','LV_SYMBOL_NEXT','LV_SYMBOL_PLAY','LV_SYMBOL_PAUSE','LV_SYMBOL_SAVE','LV_SYMBOL_MESSAGE','LV_SYMBOL_LASER','LV_SYMBOL_VACCUM','LV_SYMBOL_DISABLE_ALERT','LV_SYMBOL_LOCK','LV_SYMBOL_COOLANT','LV_SYMBOL_STOP','LV_SYMBOL_WIFI','LV_SYMBOL_WARNING','LV_SYMBOL_FOLDER','LV_SYMBOL_FILE','LV_SYMBOL_KEYBOARD','LV_SYMBOL_BACKSPACE','LV_SYMBOL_SD_CARD','LV_SYMBOL_JOG','LV_SYMBOL_UP','LV_SYMBOL_DOWN','LV_SYMBOL_LEFT','LV_SYMBOL_RIGHT','LV_SYMBOL_COMMAND','LV_SYMBOL_GAUGE','LV_SYMBOL_LANGUAGE','LV_SYMBOL_FAN','LV_SYMBOL_SPEED','LV_SYMBOL_WIZARD','LV_SYMBOL_LIGHT','LV_SYMBOL_ENGINE','LV_SYMBOL_LAYERS','LV_SYMBOL_LEVELING','LV_SYMBOL_FILAMENT','LV_SYMBOL_CENTER','LV_SYMBOL_SEARCH','LV_SYMBOL_FILAMENT_SENSOR','LV_SYMBOL_MIST','LV_SYMBOL_UNLOCK','LV_SYMBOL_LASER_2','LV_SYMBOL_MILLING','LV_SYMBOL_NEW_LINE','LV_SYMBOL_BLUETOOTH','LV_SYMBOL_USB','LV_SYMBOL_MORE_INFO','LV_SYMBOL_PLUS','LV_SYMBOL_MINUS','LV_SYMBOL_MANUAL','LV_SYMBOL_AUTOMATIC']

    # Path to the directory to search in
    directory = '../main/'

    search_defines(defines, directory, args.not_found)
