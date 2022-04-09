thrx-arduino-home-project
=========================

uses arduino to transmit data to mqtt server

Done:
* read data from easyMeter 
* send data to mqtt server for openhab / influxdb

ToDo:
* read data from room temperature sensors 
* read data from water state sensor

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


