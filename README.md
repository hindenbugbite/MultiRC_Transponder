# MultiRC IR Transponder Emulator

## Description

This project is a collection of Arduino sketches to emulate common IR timing transponders for RC/FPV racing. The goal is to allow low cost or expanded function transponders that anyone can create.

The i-Lap functions take some code from the CoreIR project. That project also has good references on building the basic hardware.

- https://github.com/RaceFPV/CoreIR

## Usage

The Arduino sketch will try to stay lean by simply sending out a predefined code. Both trackmate and i-Lap code require a CRC checksum that is calculated ahead of time by the python scripts.

Run the python script for the system you need (i-Lap/trackmate) either by editing the ID number or using interactive (-i) mode to work with the script directly. This will generate either a 4 byte or 6 byte code that is added to the sketch for compiling.


## Status

This project is on going. M5Capsule code is tested with trackmate only but will get i-Lap support soon.

## Roadmap

- Add i-Lap support 
- Add ATtiny85 support (likely a different Arduino sketch)
- If necessary, add programmable ID instead of hard coded arrays
