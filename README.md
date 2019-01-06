It is a small project that includes 125kHz RFID token reader and programmer over USB interface.

The project uses a attiny85 small test board simillar to adafruit one and a simple custom shield. The shield is not required, whole circuit could be made on a breadbord, but in case of breadboard it could be more sensitive to noise and glitches.

![Alt text](doc/rfid-reader.jpg)

Current software allows to:
- reading EM4100 tokens (16, 32, 64 carrier divider, Manchester encoding)
- writing T5557 tokens

What should be done in the near future (NOT YET IMPLEMENTED):
- support for biphase encoding,
- support for PSK encoding,
- password protection and locking (T5557),
- one-shot command to clone a token.

#### Attiny board preparation.

**!! IMPORTANT !!** Reset pin fuse should be disabled. Please keep in mind that this will disable ISP programming. Default value of fuses could be restored using AVR HV programmer only.

Demo board has a micronucleus bootloader preinstalled. It is sufficient to upgrade firmware in the atiny. No other modifications were made.

My project has no micronucleus utility included, also it is required to install it manually on OS.

```console
# cd firmware
# make clean all burn
```

#### Host software compilation.

Host software depends on libusb library. It was tested on a linux system only.

```console
# cd rfid-tool
# make clean all
```

#### Basic use cases.

```console
---- Reading a EM4100 token

# ./out/rfid-tool -r
New token, carrier divider: 64, modulation: Manchester, customer ID: 75 (0x4b), token: 1469220 (0x166b24)
New token, carrier divider: 64, modulation: Manchester, customer ID: 75 (0x4b), token: 1469220 (0x166b24)

---- Programming a T5557 token

# ./out/rfid-tool -p -b 64 -m manchester -c 0x4b -t 0x166b24


---- Reset to bootloader (firmware upgrade)

# ./out/rfid-tool -R
```
