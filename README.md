# esp-idf-rc-switch
315/433MHz ASK RF driver for esp-idf.   
There are several repositories available for esp-idf, but I couldn't find a submittable library.   
This repository can send and receive.   

I ported from [here](https://github.com/sui77/rc-switch).

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

# Hardware requirements
This project supports these wireless transceivers.   

- Transmitter   
H34A-315/H34A-433   
SYN115   
STX882   

- Receiver   
H3V3/H3V4   
SYN480R   
LR43B/LR33B   
SRX882/SRX887   

I used these transceivers.
From left: H3V4F, H34A, SYN480, SYN115   
![Image](https://github.com/user-attachments/assets/e0a27e32-cebe-457a-aaa8-11da5f5dcc31)   
![Image](https://github.com/user-attachments/assets/ef3ad73b-3aec-4a89-b507-498fb9ddbcc7)   

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-rc-switch
cd esp-idf-rc-switch
idf.py menuconfig
idf.py flash
```

# Configuration
![Image](https://github.com/user-attachments/assets/125fbaab-3e1a-4a71-ad7c-83eff94a5ba9)

## For Receiver
![Image](https://github.com/user-attachments/assets/fd511b4e-5ab4-41c4-8800-2575435c130b)

## For Transmitter
![Image](https://github.com/user-attachments/assets/187795ca-754d-4c3d-986c-dc48396207cf)

The available protocols are:   
```
static const Protocol proto[] = {
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 6 (HT6P20B)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false },    // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},     // protocol 8 Conrad RS-200 RX
  { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},      // protocol 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },     // protocol 10 (1ByOne Doorbell)
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 11 (HT12E)
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 12 (SM5212)
};

//{pulseLength, {syncFactor.high, syncFactor.low}, {zero.high, zero.low}, {one.high, one.low}, invertedSignal}
```

# Wirering

|Radio Module||ESP32|
|:-:|:-:|:-:|
|DATA|--|GPIO4|
|GND|--|GND|
|VCC|--|3.3V|

__You can change data pin to any pin using menuconfig.__   


# Communicat with Arduino Environment
I used [this](https://github.com/sui77/rc-switch) library on Arduino environment.   
The sample code for the Arduino environment can be found in the ArduinoCode folder.

### From ESP32 to UNO
![RCSwitch_Send](https://user-images.githubusercontent.com/6020549/125153827-5d7e5500-e191-11eb-8db9-accd41302b1f.jpg)

### From UNO to ESP32
![RCSwitch_Receive](https://user-images.githubusercontent.com/6020549/125153829-60794580-e191-11eb-952e-65635b412ca8.jpg)

# Using garage door 315/433MHz Remote
You can control ESP32 using garage door 315/433Mhz remote.   
You can buy it on AliExpress or eBay for about $3.   
![Image](https://github.com/user-attachments/assets/3d4ea3dc-9422-4702-96d7-3c85d7742fe2)

Build the firmware as Receiver.   
Fire the RF using the RF remote control.   
![GarageDoor](https://user-images.githubusercontent.com/6020549/166126115-506aa5c7-6fc2-49dd-9a07-53cdb6615e84.jpg)

Some example project using Garade door 315MHz/433MHz Remote.   
https://github.com/nopnop2002/esp-idf-tea5767/tree/main/withGaradeDoorRemote

# Using USB Remote Control Switch
We can obtain products that turn USB on/off with a remote control about $4.   
Both of these use a frequency of 433MHz radio signal.   
Using these, you can turn on/off the USB device from the ESP32.   
The protocols, number of bits, and codes used differ depending on the product, so please check in advance.   
You can use this project to find out what protocols, bit depths, and codes they use.   
![Image](https://github.com/user-attachments/assets/88d1a4d8-c98b-44c4-b589-0b053ff2534b)

You can turn USB on/off by sending the same protocol, number of bits, and code.   
- Products on the left
```
I (5260) MAIN: Received 16065448 / 24bit Protocol: 1
I (5360) MAIN: Received 16065448 / 24bit Protocol: 1
I (6400) MAIN: Received 16065444 / 24bit Protocol: 1
I (6500) MAIN: Received 16065444 / 24bit Protocol: 1
I (6600) MAIN: Received 16065444 / 24bit Protocol: 1
```

- Products on the right
```
I (30320) MAIN: Received 1761060 / 24bit Protocol: 1
I (30410) MAIN: Received 1761060 / 24bit Protocol: 1
I (30500) MAIN: Received 1761060 / 24bit Protocol: 1
I (30600) MAIN: Received 1761060 / 24bit Protocol: 1
```

Some example project using USB Remote Control Switch.   
https://github.com/nopnop2002/esp-idf-usb-switch

# Known Limitations   
This component uses GPIO interrupts.   
It cannot be used at the same time as other components that use GPIO interrupts.   
I tried using [this](https://components.espressif.com/components/espressif/esp32-camera) component at the same time, but it didn't work properly.   

# How to use this component in your project   
Create idf_component.yml in the same directory as main.c.   
```
YourProject --+-- CMakeLists.txt
              +-- main --+-- main.c
                         +-- CMakeLists.txt
                         +-- idf_component.yml
```

Contents of idf_component.yml.
```
dependencies:
  nopnop2002/RCSwitch:
    path: components/RCSwitch/
    git: https://github.com/nopnop2002/esp-idf-rc-switch.git
```

When you build a projects esp-idf will automaticly fetch repository to managed_components dir and link with your code.   
```
YourProject --+-- CMakeLists.txt
              +-- main --+-- main.c
              |          +-- CMakeLists.txt
              |          +-- idf_component.yml
              +-- managed_components ----- nopnop2002__RCSwitch
```
