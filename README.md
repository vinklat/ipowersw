# ipowersw
Arduino based ethernet IP power switch (relay).
Controlled by HTTP web interface or REST API. 
Includes temperature, humidity and ambient light metering for conditional operation.

## HW requirements

 * Arduino Nano rev.3 
 * ENC28J60 ethernet module
 * Relay module
 * DHTxx sensor
 * photocell

## Library requirements

 * [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor)
 * [Adafruit DHT Humidity & Temperature Sensor Library](https://github.com/adafruit/DHT-sensor-library)
 * [UIPEthernet library](https://github.com/UIPEthernet/UIPEthernet)

## HW schema

![Scheme](https://raw.githubusercontent.com/vinklat/ipowersw/master/ipowersw_scheme.png "Electrical circuit scheme")

