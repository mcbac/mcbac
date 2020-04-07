# MCBAC-V2 Hardware Changelog

**This is the changelog for the MCBAC hardware. The software changelog
is in the Software directory.**

_**NOTE:**_
 1. Versions with "alpha", "a", "beta", "b", or "rc" are pre-releases,
 and subject to change.
 2. This changelog may not include all changes, as this is hand written.

## MCBAC-V2.1-a1 - April 6, 2020

### ADDED
 - Added a `100nF` decoupling cap for `U1` (atmega328)
 - Added a trimmer to adjust the pre-regulator tracking
 - Added 2.2K to buck output
 - Added solder jumper to set the LT3083 `V_CONTROL` to `RAW_INPUT`, or `BUCK_VOUT`

### CHANGES
 - Improvement to the circuit diagram
 - Changed the `0201` caps to `0805`
 - Fixed buck converter feedback using PNP transistor
 - Changed the `Y1` (crystal) to SMD
 - Changed `R1`, and `R2` (current shunts) to `2512` SMD
 - Changed the `TEMP` read from `U8` -> `U1` (read `TEMP` from `U1`s builtin ADC)
 - Changed `PTC1`, and `PTC2` from `0201` -> `0406`
 - Changed `NTC2` from `0201` -> `0406`

### REMOVED
 - Removed unknown value resistors in parallel with `PTC1`, and `PTC2`
 - Removed the 40X20mm blower fan (maybe will use a squirrel fan)
 - Removed the unnecessary reed relay

## MCBAC-V2.0 - March 17, 2020

Basic prototype circuit.

<br>

