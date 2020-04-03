# MCBAC-V2 Hardware Changelog

**This is the changelog for the MCBAC hardware. The software changelog
is in the Software directory.**

_**NOTE:** Versions with "alpha", "a", "beta", "b", or "rc" are pre-releases,
and subject to change._

## MCBAC-V2.1-a1 April 2, 2020

### Changes

 - Improvement to the circuit diagram
 - Changed the `0201` caps to `0805`
 - Added a `10nF` decoupling cap for `U1` (atmega328)
 - Fixed buck converter feedback using PNP transistor
 - Added a trimmer to adjust the pre-regulator tracking
 - Added 2.2K to buck output
 - Added solder jumper to set the LT3083 `V_CONTROL` to `RAW_INPUT`, or `BUCK_VOUT`
 - Removed the 40X20mm blower fan (maybe will use a squirrel fan)

## MCBAC-V2.0 March 17, 2020

Basic prototype circuit.

<br>

