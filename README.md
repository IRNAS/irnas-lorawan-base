# irnas-lorawan-base
Universal code-base for multiple lorawan projects based on Arduino core. Modular structure allows the re-use of modules and features in multiple projects and thus the use of an universal code-base and regular updates.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development purposes. See Build and flash for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to use this code-base:

* [LoRa Module made by Murata](https://www.murata.com/products/connectivitymodule/lpwa/lora) on some kind of a development or prototype board.
* [Visual Studio Code](https://code.visualstudio.com/)
* [Arduino plugin for VS Code](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
* [St-link](https://www.st.com/en/development-tools/st-link-v2.html)
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)
* [ArduinoCore for STM32L0](https://github.com/IRNAS/ArduinoCore-stm32l0)
* Installed Arduino libraries: LibLacuna-0.96, [Adafruit BME280 library](https://github.com/adafruit/Adafruit_BME280_Library), [Adafruit Sensor library](https://github.com/adafruit/Adafruit_Sensor)

### Installing

* Install Arduino IDE, make sure that you click on Windows installer, not on Windows app.
* Install ArduinoCore for STM32L0, follow installation rules [here](https://github.com/IRNAS/ArduinoCore-stm32l0)
* Install Visual Studio Code and Arduino plugin 
* clone our repository with `git clone https://github.com/IRNAS/irnas-lorawan-base` and open main directory in VS Code

## Secrets
Put the LoRa keys in `secrets.h` file

    #define RELAY_NETWORKKEY[] = {...};
    #define RELAY_APPKEY[] = {...};
    #define RELAY_DEVICEADDRESS[] = {...};


## Build and flash 

* Connect St-link to LoRa module and PC
* In VS Code, bottom right click select board and from select board dropdown menu select `IRNAS-env-module-L072Z` and close configuration tab
* Click Arduino: Upload button in top right, code will be compiled and flashed to the board.

## Using release_build.py script

'Release_build.py' will create a version.h file with provided tag, do a commmit and push and automatically create a build release on GitHub.

### Prerequisites

Github_release python module is needed, it can be installed by running:
```
pip install githubrelease
```
### Usage 
Introductory message should be self explanatory, an example command should look something like this:
```
python release_build.py --version v.1.23.5 --body "Here is body of release message" --branch master
```
## Authors

* **Luka Mustafa** - *Initial work, ongoing work* - [SloMusti](https://github.com/SloMusti)
* **Marko Sagadin** - *documentation, ongoing work* - [SkobecSlo](https://github.com/SkobecSlo)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.
