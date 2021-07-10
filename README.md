# esp-idf-rc-switch
433/315Mhz RF driver for esp-idf.   
There are several repositories available for esp-idf, but I couldn't find a submittable library.   
This repository can send and receive.   

I ported from [here](https://github.com/sui77/rc-switch).

# Hardware requirements
315/433MHz AM transmitter and one or more devices with one of the supported chipsets:
- SC5262 / SC5272
- HX2262 / HX2272
- PT2262 / PT2272
- EV1527 / RT1527 / FP1527 / HS1527
- Intertechno outlets
- HT6P20X

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

# Communicat with AtMega/STM32/ESP8266/ESP8285
I used [this](https://github.com/sui77/rc-switch) library on Arduino environment.

![RCSwitch_Send](https://user-images.githubusercontent.com/6020549/125153827-5d7e5500-e191-11eb-8db9-accd41302b1f.jpg)

![RCSwitch_Receive](https://user-images.githubusercontent.com/6020549/125153829-60794580-e191-11eb-952e-65635b412ca8.jpg)

