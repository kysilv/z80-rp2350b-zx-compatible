# z80-rp2350b-zx-compatible
This is a retrocomputer compatible with ZX Spectrum and based on real Z80 + discrete RAM/ROM + rp2350b to cover ULA functionality programmatically.

## Specification:

**CPU**: Z84C00AB6 Z80A CPU DIP-40

**ROM**: W27c512-45z DIP-28

**RAM**: 2xHM62256ALP-10 DIP-28

**ULA**: WeAct RP2350B Core board

**Video**: VGA (DE-15) 720x576p emulating Spectrum PAL resolution, HDMI (not implemented yet).

**Audio**: Speaker on board

**Cassette interface**: Jack 3.5mm for smartphone headset jack

**Keyboard**: Handmade

**Joystick**: 
  1. Retro DA-15 GamePort joystick interface connected to expansion connector, emulating Kempston port 1F.
  2. Sinclair Joystick 1 in parallel to keyboard 6-0 (connector not soldered yet).

## Folders:

**firmware**: rp2350b firmware sources 

**hardware**: Pictures of schematic and pcbs. KiCAD project is not ready yet, has bugs and issues (see photos of fixed pcb in Photo folder) so only pictures for now until the project fixed and ready.

**photos**: Photos of the device in process of tuning and some "screenshots".
