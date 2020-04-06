# MCBAC-V2 Hardware Changelog

**This is the changelog for the MCBAC hardware. The software changelog
is in the Software directory.**

_**NOTE:**_
 _1. Versions with "alpha", "a", "beta", "b", or "rc" are pre-releases,_
and subject to change.
 _2. This changelog may not include all changes, as this is hand written._

## MCBAC-V2.1-a1 April 6, 2020

### Changes

 - Improvement to the circuit diagram
 - Changed the `0201` caps to `0805`
 - Added a `100nF` decoupling cap for `U1` (atmega328)
 - Fixed buck converter feedback using PNP transistor
 - Added a trimmer to adjust the pre-regulator tracking
 - Added 2.2K to buck output
 - Added solder jumper to set the LT3083 `V_CONTROL` to `RAW_INPUT`, or `BUCK_VOUT`
 - Removed the 40X20mm blower fan (maybe will use a squirrel fan)
 - Removed the unnecessary reed relay
 - Changed the `Y1` (crystal) to SMD
 - Changed `R1`, and `R2` (current shunts) to `2512` SMD
 - Changed the `TEMP` read from `U8` -> `U1` (read `TEMP` from `U1`s builtin ADC)
 - Removed unknown value resistors in parallel with `PTC1`, and `PTC2`
 - Changed `PTC1`, and `PTC2` from `0201` -> `0406`
 - Changed `NTC2` from `0201` -> `0406`

## MCBAC-V2.0 March 17, 2020

Basic prototype circuit.

<br>

