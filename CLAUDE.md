# LG IR Remote — Arduino Pro-Mini 3V3

## Project goal
Build an IR remote for an LG Smart TV using an Arduino Pro-Mini 3V3 board powered by a single Li-ion cell. The output is:
- A single Arduino `.ino` sketch file.
- A text-based diagram
- a png diagram
- A Readme file for Github

## Hardware
- Arduino Pro-Mini 3V3 (ATmega328P, 8 MHz)
- IR LED (940 nm) driven via 2N2222 NPN transistor (1 kΩ base resistor, 100Ω emitter resistor)
- 4 momentary push buttons, wired between pin and GND (internal pull-ups, active LOW)
  - Power toggle
  - HDMI-1
  - HDMI-2
  - HDMI-3
- Powered by 3.7V Li-ion cell connected directly to VCC (bypass regulator)
- Remove onboard voltage regulator and power LED to minimise quiescent current

## Pin assignments
| Pin | Function |
|-----|----------|
| D3  | 2N2222 base via 1 kΩ (OC2B — Timer2 PWM for 38 kHz carrier) |
| D4  | Power toggle button → GND |
| D5  | HDMI-1 button → GND |
| D6  | HDMI-2 button → GND |
| D7  | HDMI-3 button → GND |
| VCC | 3.7 V Li-ion (+) — regulator removed |
| GND | 3.7 V Li-ion (−) |

## IR protocol — LG NEC
- Protocol: NEC, 32-bit, 38 kHz carrier
- Device address: `0x04`, inverted: `0xFB`
- Frame format: `(addr)(~addr)(cmd)(~cmd)`
- Commands: Power `0x08`, HDMI-1 `0xCE`, HDMI-2 `0xCC`, HDMI-3 `0xE9`

## Power saving
- Use `SLEEP_MODE_PWR_DOWN` between button presses 
- Wake via pin-change interrupt on any of the 4 button pins
- Disable any unnessesary features, to save power
- Add debouncing

## Collaboration
- Ask clarifying questions before making assumptions about hardware, protocol, or design choices
- When multiple valid approaches exist, present options and let the user decide

## Sketch requirements
1. Single `.ino` file: `lg_ir_remote/lg_ir_remote.ino`
2. Comment block at top:
   - Board names
   - Libraries
   - Full wiring summary
3. All constants (pins, addresses, commands) named and grouped at the top
4. No magic numbers anywhere in the code

## Git & GitHub workflow
Create a new project on Github

### Branching
- No branching; all commits are automaticaly merged to 'main' 
- Commit to main only when the sketch compiles cleanly

