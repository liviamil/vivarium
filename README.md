# Vivarium

Vivarium is an Arduino-based emotional communication device that enables users to record voice messages while receiving visual emotional feedback through an OLED display. It serves as a prototype for a conceptual device that creates emotionally safe spaces for communication when direct conversation isn't possible.

## Project Background
Vivarium was developed as a critical design project exploring how technological interventions might create spaces for emotional expression when direct communication is impossible or difficult. The device serves as both a functional prototype and a cultural probe to investigate how people address unresolved emotional needs.

## Features
1. Voice recording to microSD card in WAV format
2. Visual emotional feedback via changing emoji expressions on OLED display
3. Simple one-button operation for starting/stopping recording
4. LED indicator showing recording status
5. Portable, standalone operation (no computer required after setup)

## Components Required

| **Component**       | **Description**                           | **Quantity**           |
|---------------------|-------------------------------------------|------------------------|
| Arduino Mega        | Microcontroller board                     | 1                      |
| MAX4466             | Microphone Module    | 1                      |
| Pololu sdc01a       | MicroSD Card Module                       | 1                      |
| KODAK MicroSD       | Storage for audio files (any brand will work) | 1                  |
| OLED 128x64         | Display Module (I2C interface)            | 1                      |
| Button          | For recording control                     | 1                      |
| LEDs                | Status indicators (recording active)      | 2                      |
| Resistors           | 220Ω for LEDs                             | 2                      |
| Jumper Wires        | For connections                           | Various                |
| Breadboard          | For prototyping                           | 1                      |

## Pin Connections

### Microphone (MAX4466)
VCC → 5V on Arduino
GND → GND on Arduino
OUT → A0 on Arduino

### MicroSD Card Module (Pololu sdc01a)
GND → GND on Arduino
VDD → 3.3V on Arduino
DI → Pin 51 on Arduino
DO → Pin 50 on Arduino
CS → Pin 52 on Arduino
CD → Pin 32 on Arduino

### OLED Display (128x64 I2C)
SDA → Pin 20 on Arduino
SDL → Pin 21 on Arduino
GRD → GND on Arduino 
VOLT → 5V on Arduino 

### Button & LEDs
5V → Button → Resistor → Pin 26 on Arduino → Indicator LED → GND
Pin 24 on Arduino → Resistor → Indicator LED 
