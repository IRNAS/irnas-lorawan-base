#!/usr/bin/python
"""
This code automatically fetches .h files which contain data structures representing binary data.
A template json file is created with the deat structure representing bytes and values.
User can then populate this template with default values or otherwise and provide settings to the device.
IMPORTANT: Template must be rebuilt by running this script each time something changes.

Two files are created: 
settings_template.json - system file used for data encoding
settings_sample.json - sample file to edit and insert custom settings
"""
import sys
import CppHeaderParser
import json

# This just makes the output more human readable
def collapse_json(text, indent=12):
    """Compacts a string of json data by collapsing white space after the
    specified indent level

    NOTE: will not produce correct results when indent level is not a multiple
    of the json indent level
    """
    initial = " " * indent
    out = []  # final json output
    sublevel = []  # accumulation list for sublevel entries
    pending = None  # holder for consecutive entries at exact indent level
    for line in text.splitlines():
        if line.startswith(initial):
            if line[indent] == " ":
                # found a line indented further than the indent level, so add
                # it to the sublevel list
                if pending:
                    # the first item in the sublevel will be the pending item
                    # that was the previous line in the json
                    sublevel.append(pending)
                    pending = None
                item = line.strip()
                sublevel.append(item)
                if item.endswith(","):
                    sublevel.append(" ")
            elif sublevel:
                # found a line at the exact indent level *and* we have sublevel
                # items. This means the sublevel items have come to an end
                sublevel.append(line.strip())
                out.append("".join(sublevel))
                sublevel = []
            else:
                # found a line at the exact indent level but no items indented
                # further, so possibly start a new sub-level
                if pending:
                    # if there is already a pending item, it means that
                    # consecutive entries in the json had the exact same
                    # indentation and that last pending item was not the start
                    # of a new sublevel.
                    out.append(pending)
                pending = line.rstrip()
        else:
            if pending:
                # it's possible that an item will be pending but not added to
                # the output yet, so make sure it's not forgotten.
                out.append(pending)
                pending = None
            if sublevel:
                out.append("".join(sublevel))
            out.append(line)
    return "\n".join(out)

def fetch_settings_struct_from_header(filename,classname):
    try:
        cppHeader = CppHeaderParser.CppHeader(filename)
    except CppHeaderParser.CppParseError as e:
        #print(e)
        print("File " + filename + " not found")
    return cppHeader.classes[classname]["properties"]["public"]

def create_array_from_settings(settings):
    count = 0
    length = 0
    values={}
    for item in settings:
        values[item["name"]]={}
        values[item["name"]]["value"]=0
        length_bytes=validate_decode_variable_type(item["type"])
        values[item["name"]]["size"]=length_bytes
        length+=length_bytes
        values[item["name"]]["order"]=count
        count+=1
    values["length"]["value"]=length
    return values

def validate_decode_variable_type(type):
    #valid datatype lengths
    data={"uint8_t":1,"uint16_t":2,"int8_t":1,"int16":2, "uint32_t":4, "uint64_t":8}
    try:
        return data[type]
    except:
        print("Incorrect data type, error! " + type)


#create json template of all the settings

template_settings={}

### basic device settings
settings = fetch_settings_struct_from_header("src/settings.h","settingsData_t")
basic_settings=create_array_from_settings(settings)
# add some data manually
basic_settings["global_id"]["value"]=1
#append to the template
template_settings["basic_settings"]=basic_settings

### module_system settings
settings = fetch_settings_struct_from_header("src/module_system.h","MODULE_SYSTEM::module_settings_data_t")
module_system=create_array_from_settings(settings)
# add some data manually
module_system["global_id"]["value"]=2
#append to the template
template_settings["module_system"]=module_system

### module_gps_ublox settings
settings = fetch_settings_struct_from_header("src/module_gps_ublox.h","MODULE_GPS_UBLOX::module_settings_data_t")
module_gps_ublox=create_array_from_settings(settings)
# add some data manually
module_gps_ublox["global_id"]["value"]=3
#append to the template
template_settings["module_gps_ublox"]=module_gps_ublox

### module_pira settings
settings = fetch_settings_struct_from_header("src/module_pira.h","MODULE_PIRA::module_settings_data_t")
module_pira=create_array_from_settings(settings)
# add some data manually
module_pira["global_id"]["value"]=4
#append to the template
template_settings["module_pira"]=module_pira

### module_accelerometer settings
settings = fetch_settings_struct_from_header("src/module_accelerometer.h","MODULE_ACCELEROMETER::module_settings_data_t")
module_accelerometer=create_array_from_settings(settings)
# add some data manually
module_accelerometer["global_id"]["value"]=5
#append to the template
template_settings["module_accelerometer"]=module_accelerometer

# creates the template with lengths and default values
f = open('settings_template.json', 'w')
f.write(collapse_json(json.dumps(template_settings, indent=4),indent=8))
f.close()

# drop length and position from template
for module in template_settings:
    for setting in template_settings[module]:
        print(template_settings[module][setting])
        del template_settings[module][setting]["size"]
        del template_settings[module][setting]["order"]
        default_value=template_settings[module][setting]["value"]
        # collapse the value
        del template_settings[module][setting]["value"]
        template_settings[module][setting]=default_value
        # hide global id and lengths as they can not be user modified
    del template_settings[module]["global_id"]
    del template_settings[module]["length"]



#creates the sample config which is user-editable
f = open('settings_sample.json', 'w')
f.write(collapse_json(json.dumps(template_settings, indent=4),indent=8))
f.close()

print(template_settings)
