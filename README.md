# Corona-Exposure-Notification-Scanner

![Scan](https://github.com/KoenVdH/Corona-Exposure-Notification-Scanner/blob/main/screen3.jpg)
![Scanning](https://github.com/KoenVdH/Corona-Exposure-Notification-Scanner/blob/main/scan.gif)

M5STACK ESP32 Arduino

This radar app shows "covid trace" beacons transmitted around you.
The circle distances from the centre are related to the strength of the received signals.

THESE ARE NOT infections but people that have the corona alert app installed.
Also the visualisation is intended to look good not to show the location of other people.

Works with the standard BLE Arduino lib that is installed with Arduino 1.9.

https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf?1

Versions 2 and 3 other visualisations. I try to make it look as cool as possible :-)

Known issues: 

Crash when receiving too much input. E.g. when used in places with lots of people...
