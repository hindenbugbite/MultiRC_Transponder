# MultiRC IR Transponder Emulator

## Description

This project is a collection of Arduino sketches for several microcontrollers to emulate common IR timing transponders for RC/FPV racing. The goal is to allow low cost or expanded function transponders that anyone can build on.

Trackmate IR code was decoded using a BusPirate to sample the code from several transponders. 

The i-Lap functions take some code from the CoreIR project. That project also has good references on building the basic hardware.

- https://github.com/RaceFPV/CoreIR

## Usage

The Arduino sketch will try to stay lean by simply sending out a predefined code. Both trackmate and i-Lap code require a CRC checksum that is calculated ahead of time by the python scripts.

Run the python script for the system you need with a command line argument for the ID you want (Trackmate is 4 digits/i-Lap is 7 digits). This will generate either a 4 byte or 6 byte code that is copied to the sketch for compiling.

The M5Capsule can cycle through any number of codes with the top button input with buzzer feedback when the code list repeats.

The ATtiny85 assumes micronucleus bootloader and uses the ATtinyCore library.

## Logic Analyzer Traces

Both Trackmate and i-Lap transponders were sampled using a BusPirate5 and PulseView. These are saved for future reference in the PulseView folder.

## Status

This project is on going. M5Capsule code is tested with trackmate only but will get i-Lap support soon. ATtiny85 code is waiting to be tested.

## Roadmap

- Add i-Lap support to ATtiny85 
- Add ID switching to ATtiny85
- If necessary, add programmable ID instead of hard coded arrays
