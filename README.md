# Use Flash memory as EEPROM

With the ESP32 and the EEPROM library you can use up to 512 bytes in the flash memory.

## Implementation

The data stored in the flash memory will not disappear when the power is off.

In this implementation we will use it to store variables that must be memorized：

* uvTime
* rfAddrID
* fanGear

We will use 17 bytes of flash memory space to store the data.
