# esp-idf-rc-switch
433/315Mhz RF driver for esp-idf.   
There are several repositories available for esp-idf, but I couldn't find a submittable library.   
This repository can send and receive.   

I ported from [here](https://github.com/sui77/rc-switch).

# Hardware requirements
315/433MHz AM transmitter and AM receiver with one of the supported chipsets:
- SC5262 / SC5272
- HX2262 / HX2272
- PT2262 / PT2272
- EV1527 / RT1527 / FP1527 / HS1527
- Intertechno outlets
- HT6P20X

I used this:

![H3V4F-2](https://user-images.githubusercontent.com/6020549/125154180-c36bdc00-e193-11eb-9e89-5934c2db7fae.JPG)
![H3V4F-3](https://user-images.githubusercontent.com/6020549/125154246-0ded5880-e194-11eb-974b-03232166ab12.JPG)

__Note__   
As far as I know, there are 5V module and 3.3V module in the receiving module.   
__Only 3.3V__ modules can be used with the ESP32.   


# Installation for ESP32
```
git clone https://github.com/nopnop2002/esp-idf-rc-switch
cd esp-idf-rc-switch
idf.py set-target esp32
idf.py menuconfig
idf.py flash
```

# Installation for ESP32-S2
```
git clone https://github.com/nopnop2002/esp-idf-rc-switch
cd esp-idf-rc-switch
idf.py set-target esp32s2
idf.py menuconfig
idf.py flash
```

# Configuration
![config-main](https://user-images.githubusercontent.com/6020549/125153763-fc568180-e190-11eb-89b4-1aea05542101.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/125153764-fe204500-e190-11eb-9cbf-a56ab0faa0ce.jpg)
![config-direction](https://user-images.githubusercontent.com/6020549/125153778-12644200-e191-11eb-98c5-692739a1d6bc.jpg)

On the transmitting side, you can choose the protocol and the number of data bits.
![config-transmit](https://user-images.githubusercontent.com/6020549/125153790-260fa880-e191-11eb-873b-4b975af293d0.jpg)

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

|Radio Module||ESP32/ESP32S2|
|:-:|:-:|:-:|
|DATA|--|GPIO22|
|GND|--|GND|
|VCC|--|3.3V|

__You can change data pin to any pin using menuconfig.__   
__However, changing to some pins does not work properly.__


# Communicat with AtMega/STM32/ESP8266/ESP8285
I used [this](https://github.com/sui77/rc-switch) library on Arduino environment.

## From ESP32 to UNO
![RCSwitch_Send](https://user-images.githubusercontent.com/6020549/125153827-5d7e5500-e191-11eb-8db9-accd41302b1f.jpg)

## From UNO to ESP32

![RCSwitch_Receive](https://user-images.githubusercontent.com/6020549/125153829-60794580-e191-11eb-952e-65635b412ca8.jpg)

