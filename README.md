# Gladys radioemitter

Emitter 433 IR is used to control radio plugs and IR materials.

## Installation

- First, install the [Serial](https://developer.gladysproject.com/en/modules/serial) module in Gladys
- Then, install this module in Gladys
- Push the [arduino code](https://github.com/billonalex/gladys-emitter433IR/blob/master/arduino-code/arduino-code.ino) to your arduino with the [Arduino IDE](https://www.arduino.cc/en/main/software). You need to have `ArduinoJson` and `RCSwitch` libraries installed. To install the libraries, in the arduino IDE (version > 1.6) go to "Sketch" => "Include Library" => "Manage Libraries". Then search "ArduinoJson" and click on "install". Same for "RCSwitch".
- Reboot Gladys
- Connect your arduino in USB to your Raspberry Pi
- Click on the "config" button of the "Serial" module in the "Modules" view. 
- Create a device in Gladys in the Device view with the following info => 

| name | identifier | protocol | service 
| ---| ---| ---| ---| 
| My Switch | YOUR_DECIMAL_CODE_FOR_THIS_SWITCH_OFF | "THE_FUNCTION_TO_BE_CALLED_BY_ARDUINO" | radioemitter 

- Then, inside this device, create a deviceType :

| type | min | max 
| ---| ---| ---|
| binary | 0 | 1

or

| type | min | max 
| ---| ---| ---|
| push | 0 | 1

## Credits

Thanks to [Irumi](https://community.gladysproject.com/u/irumi/summary) for the first version of this module :)
