import json
import struct
import binascii

"""
Encoder takes the settings and templates and creates the binary encoded payload to be sent to the device.
Settings are prepared for all the defined modules, if any values are undefined or none, then default values are used

Two files read: 
settings_template.json - system file used for data encoding
settings_sample.json - sample file to edit and insert custom settings

Output:
binary encoded payload - copy paste to TTN
"""

with open('../settings_sample.json') as json_file:
    settings = json.load(json_file)
    #print(settings)

with open('../settings_template.json') as json_file:
    template = json.load(json_file)
    #print(template)


processed = {}
# enter values from settings into the template
try:
    for module in settings:
        # create output only for defined modules
        processed[module]=template[module]
        for setting_value in settings[module]:
            if settings[module][setting_value] is not None:
                processed[module][setting_value]["value"]=settings[module][setting_value]
except KeyError:
    print("Incorrect settings, check the sample versus template.")

# perform binary encoding
# force split into multiple packets if payload is longer then 51 bytes
# TODO: enforce correct ordering, currently implicitly defined through order in json
bytes_to_fmt={1:"B",2:"H",4:"I",8:"Q"}

binary_output=[bytes()]
packet_counter=0
expected_length=[0]
for module in processed:
    module_length=processed[module]["length"]["value"]
    if((expected_length[packet_counter]+module_length)>51):
        packet_counter+=1
        binary_output.append(bytes())
        expected_length.append(0)
        expected_length[packet_counter]=processed[module]["length"]["value"]
    else:
        expected_length[packet_counter]+=processed[module]["length"]["value"]
    for setting_value in processed[module]:
        binary_output[packet_counter]+=struct.pack(bytes_to_fmt[processed[module][setting_value]["size"]],int(processed[module][setting_value]["value"]))

for count in range(len(binary_output)):
    if(len(binary_output[count])!=expected_length[count]):
        print(str(len(binary_output[count])) + " " + str(expected_length[count]))
        print("Packet generation error!")
    else:
        print("Packet " + str(count+1) + " with " + str(len(binary_output[count])) + " bytes: " + str(binascii.b2a_hex(binary_output[count])))


