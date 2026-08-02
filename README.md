# 🌱 Smart Irrigation System using ESP8266

An IoT based Smart Irrigation System using ESP8266, Firebase, DHT11 and Soil Moisture Sensor.

## Features

- Automatic irrigation
- Soil moisture monitoring
- Temperature monitoring
- Humidity monitoring
- Firebase Realtime Database
- Remote pump control
- Water saving logic

## Hardware

- ESP8266 NodeMCU
- Soil Moisture Sensor
- DHT11
- L298N Motor Driver
- 9V Water Pump
- Power Supply

## Software

- Arduino IDE
- Firebase
- ESP8266WiFi Library

## Circuit

<img width="845" height="714" alt="image" src="https://github.com/user-attachments/assets/776eb943-c9e5-4b2f-9309-3f29a80dc5c1" />


## Working

1. Read soil moisture
2. Read temperature and humidity
3. Upload values to Firebase
4. If soil is dry:
   - Turn ON pump
5. Otherwise:
   - Turn OFF pump

## Firebase Structure

```
Moisture
Temperature
Humidity
Pump-status
```

## Author

Sreenandh
