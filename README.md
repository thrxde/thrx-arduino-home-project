# thrx-arduino-home-project
=========================

uses arduino to transmit data to mqtt server

## Protocol:

### EasyMeter example:

// /ESY5Q3DA1024 V3.03
//
// 1-0:0.0.0*255(112940679)
// 1-0:1.8.0*255(00001013.0368091*kWh) //Bezugsregister kWh // length 35
// 1-0:2.8.0*255(00000376.0128508*kWh) //Lieferregister kWh // length 35
// 1-0:21.7.255*255(000046.04*W) //Momentanleistung-L1 W // length 29
// 1-0:41.7.255*255(000122.30*W) //Momentanleistung-L2 W // length 29
// 1-0:61.7.255*255(000079.03*W) //Momentanleistung-L3 W // length 29
// 1-0:1.7.255*255(000247.37*W)  //Momentanleistung- L1 - L3 W // length 29
// 1-0:96.5.5*255(82)  //StatusHex // length 18
// 0-0:96.1.255*255(1ESY1233002534)
// !
//

### EasyMeter protocol:

// 1-0:1.8.0*255 //Bezugsregister kWh // length 16
// 1-0:2.8.0*255 //Lieferregister kWh // length 16

// 1-0:21.7.255*255 //Momentanleistung-L1 W // length 9
// 1-0:41.7.255*255 //Momentanleistung-L2 W // length 9
// 1-0:61.7.255*255 //Momentanleistung-L3 W // length 9
// 1-0:1.7.255*255 //Momentanleistung- L1 - L3 W // length 9

// 1-0:21.7.0*255 //Momentanleistung-L1 W // length 9
// 1-0:41.7.0*255 //Momentanleistung-L2 W // length 9
// 1-0:61.7.0*255 //Momentanleistung-L3 W // length 9
// 1-0:1.7.0*255 //Momentanleistung- L1 - L3 W // length 9

## Status:

[x] read data from easyMeter 
[x] send data to mqtt server for openhab / influxdb
[x] validate data from easyMeter
[] read data from water state sensor
[] read data from room temperature sensors


## pio commands:

Update:

adjust private_config.ini
```
pio run
pio run --target upload
```

Monitor: 
```
pio device monitor
```

Upgrade pio:
```
pio update
pio upgrade
```


```
pio init
pio settings
pio boards
```

## Troubleshooting:

Error:
```
could not open port '/dev/ttyACM0': [Errno 13] could not open port /dev/ttyACM0: [Errno 13] Permission denied: '/dev/ttyACM0'
```

Is Arduino connected?
```
lsusb
```

Solution:
```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/master/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules

sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
sudo usermod -a -G uucp $USER
sudo usermod -a -G lock $USER
```


