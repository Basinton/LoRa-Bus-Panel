# Bus Notify Panel - LoRa Board

## Overview
The LoRa Board allows users to call buses through push buttons and corresponding led lights.

## Setup
1. **PlatformIO Installation**  
   Install PlatformIO and the necessary libraries.

2. **Dependencies**  
   - Platform: espressif32
   - Board: esp32dev
   - Library:
     + LoRa_E32.h
     + TinyGPSPlus.h
     + SoftwareSerial.h

   Ensure these dependencies are listed in `platformio.ini`.

3. **Wiring**
   - Module Lora:
     + PIN_M0    Pin 5
     + PIN_M1    Pin 18
     + PIN_AUX   Pin 4
4. **Configuration**  
   Read the `.h` file for configuration information.

## Usage
1. **Upload Firmware**  
   Upload the firmware using PlatformIO.

2. **Operation**  
   The control panel will display lights when someone presses the button and has 7-segment LED lights to display the number of people calling the bus.

3. **Troubleshooting**
   - Check the wire connections of the display panel.
   - Ensure adequate power supply.
