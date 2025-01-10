# pump_controller

A simple program that runs on Arduino Uno R4 Wifi to control a VFD (variable frequency drive) which in turn runs a deep (water) well submersible pump.

(c) 2025 Ben McCart

Release under the MIT license, see "license.txt" for details.


Problem Solved
==============
I have a deep water well with a 5 HP submersible pump.  To grant better motor/pump life I have opted to use a VFD.  Initially I had used the built-in software based PLC (programmable ladder logic) to control the application but ended up having some hardware trouble after a season.

The replacement was to control the VFD using a SBC (single-board-computer) and some very basic C++ code with the available libraries.  My VFD is a TECO A510.  My SBC is an Arduino Uno R4 Wifi.  The wifi function is not used but the led matrix is used to display status messages including error codes.

Implementation
==============
For the project I ended up using C++17 and PlatformIO.  I had to manually copy the Matrix library from the Arduino IDE instead of using the dependency management in PlatformIO, as the version of the library it retrieved didn't support asynchronous output to the LED matrix, as of this writing.

I am using as SD card reader to read a JSON config file for the relavant control and input register from VFD, but you could configure the JSON file to specifiy an analog input on the Arduino, as that capability was implemented.  I am doing the same for a flood sensor (detect submersion in liquid) which should prevent significant flooding in the event that pressure transducer fails and overpumping opens the overpressure valve on the water pressure tank.

The VFD is controlled with RS-485 modbus.  Any RS485 card that works with serial port over standard UART should work with this program on the Arduino Uno R4 Wifi.

In theory, any VFD that can be controlled via RS-485 modbus should be supportable, you just have to know what the registers are for run/stop and frequency, and how those values (run/stop & frequency) are represented on your VFD.

The logic currently support two-stage fill.  There is a bottom low pressure at which the pump turn on, and then above a middle pressure the pump slows to a slower fill speed.  When the full pressure is readed the pump shuts off.  The two stage design is to minimize pumping start cycles when there are is high volume consumption for extended periods.

I am sharing this in the hope that perhaps it will prove useful to others who might have the same problem to solve.

 
    
