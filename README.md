# irnas-lorawan-base
Universal code-base for multiple lorawan projects based on Arduino core. Modular structure allows the re-use of modules and features in multiple projects and thus the use of an universal code-base and regular updates.

### Downlink encoding
To prepare data for the downlinks to happen, there is a json template `encoded_settings_template.json` which is created using `encoder_generator.py` each time fields the device sends change in the development process.

The structure outline is as follows:
 * `module name`
   * `setting name`
     * `value` - the value of the setting - to be modified by the user
     * `size` - the variable size in bytes - to be used by the encoder
     * `order` - order of the value in the payload - to be used by the encoder
    
The user can send the settings for each module individually or for multiple modules at the same time.

TTN downlink `encoder.js` is written in javascript and is an universal piece of code, autoamtically generating the message based on the json input which contains all the required information