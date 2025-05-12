# MAE412 Final Project

This project provides a complete hardware and software solution for testing and controlling a model train system using both a custom vector board (6502 assembly) and an Arduino. It includes firmware, isolated test programs, schematics, and 3D-printable mechanical parts.

## Directory Structure

- `testStand.asm` — Main assembly code for the vector board (final code for test stand)
- `testStandArduino/testStandArduino.ino` — Main Arduino code (final code for Arduino)
- `isolatedTests/` — Standalone test programs for individual hardware components (servos, relays, sensors, etc.)
- `sch_files/` — Schematics for the vector board (PDF and .sch formats)
- `stl_files/` — 3D-printable mechanical parts for the test stand
- `full/` — Additional/archived versions of assembly and Arduino code

## Main Components

### 1. Vector Board (6502 Assembly)
- **File:** `testStand.asm`
- Implements the main finite state machine (FSM) for train control
- Handles communication with the Arduino via ACIA
- Controls track power, train movement, and interfaces with sensors

### 2. Arduino
- **File:** `testStandArduino/testStandArduino.ino`
- Receives commands from the vector board
- Controls servos for pickup/dropoff, relays for track power, and reads sensors (Hall effect, color sensor)
- Implements routines for marble pickup, dropoff, and track switching

## How It Works

- The vector board runs the main FSM, sending commands to the Arduino to perform physical actions (move servos, switch relays, etc.).
- The Arduino executes these commands and returns status/data as needed.
- The system is designed for modular testing and can be extended or debugged using the isolated test programs.

## Getting Started

### Requirements
- 6502-based vector board (or emulator)
- Arduino (compatible with SoftwareSerial, e.g., Uno)
- Required sensors: Hall effect, APDS-9960 color sensor
- Servos and relays as per pin definitions in Arduino code

### Setup
1. **Vector Board:**
   - Assemble and flash `testStand.asm` to your vector board.
2. **Arduino:**
   - Upload `testStandArduino/testStandArduino.ino` to your Arduino.
   - Connect sensors and actuators as per pin definitions in the `.ino` file.
3. **Wiring:**
   - Connect the vector board and Arduino via serial (see code comments for pinout).
4. **Schematics:**
   - Reference `sch_files/Vector Board Schematic v13.pdf` for wiring and hardware setup.
5. **Mechanical Parts:**
   - Print parts from `stl_files/` as needed for your test stand.

### Usage
- Power on both systems. The vector board will initialize and communicate with the Arduino to begin the test sequence.
- Use the isolated test programs in `isolatedTests/` to debug or verify individual hardware components before full integration.

## Isolated Tests
- `isolatedTests/servoTest/servoTest.ino` — Test servo operation
- `isolatedTests/hallTest/hallTest.ino` — Test Hall effect sensor
- `isolatedTests/colorTest/colorTest.ino` — Test color sensor
- `isolatedTests/relayTest/relayTest.ino` — Test relay switching
- `isolatedTests/trickleTest/trickleTest.ino` — Test trickle mechanism
- `isolatedTests/colorServoTest/colorServoTest.ino` — Combined color and servo test
- ...and more for advanced debugging

## Schematics and Mechanical Files
- **Schematics:**
  - `sch_files/Vector Board Schematic v13.pdf` (latest)
  - `.sch` files for editing in schematic software
- **3D Parts:**
  - `stl_files/` contains all STL files for mechanical assembly (e.g., Pusher, RelayMount, MarbleCarrier, Legs, etc.)

## Authors & Credits
- Jorrel Rajan (main author, software)
- Group: Jorrel Rajan, Jake Vazquez and Eliza Brown

---

For more details, see comments in the main code files and the schematics. For questions or contributions, please contact the project maintainer.
