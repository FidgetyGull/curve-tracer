# Simple Semiconductor Curve Tracer
## About
This project is my first experience with semiconductor devices. This project involves ptting together an ADS1115 ADC and MCP4725
DAC to measure I-V characteristics

### Current Stage
- Finsished first breadboard prototype
- Built both apps
- Plotted first diode (1N4148)

### Next steps:
- Device characterization
- More diodes/LEDs/etc
- Transistors

## Instructions
1. Create Python virtual environment and install requirements
2. Flash C++ app to esp32 (using Arduino ide, PlatformIO, etc)
3. Run `read_curve.py` Graphs will appear after a few seconds, depending on how many data points are collected

### Notes
1. Make sure your serial port is correct
2. Make sure baud rate is the same between esp32 app and python script
3. esp32 should be plugged in before running