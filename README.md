# rg35xx-mtp-server
 
Chopped and duct taped fork of KDE fork of mtp-server adapted for
RG35XX GarlicOS 1.x.x that makes USB port
have working MTP connection for file transfers

/dev/mtp_usb must be available, this usually is present on android kernels
that have mtp usb gadget function like on this case, so could potentially
work on other devices after some adaptations or recompiling.

## Installation
Each folder in artifacts represents the partition to copy the files into
 
### - MISC:
Copy the files and edit `dmenu.bin` in MISC to call `/misc/usb.sh` instead of:
```
    # Enable ADB for debugging
    if [ -f /misc/enableADB ]
    then
        /usbdbg.sh device
    fi
```
This script will setup USB to device mode if MTP and/or ADB is enabled

ADB with MTP or only ADB is allowed too

`/misc/enableMTP` must exist to enable MTP functionality

### - ROMS

Copy the files, by default both TF1 and TF2 are exposed but can be edited in `CFW/mtp/run.sh`

MTP_ENTRY_LEN specifies how many entries will be declared in env's, check file for examples

A optional APPS script is provided to toggle MTP by creating/removing `/misc/enableMTP`

## Compiling

As rg35xx toolchain lacks some boost modules required to compile this a fork was made to add them:
https://github.com/IonAgorria/rg35xx-toolchain

compile.sh can be called inside the toolchain shell to compile the binary, the result will be placed into artifacts

## Disclamer

This hasn't been tested extensively and may have issues, testing with 1.5GB files worked so far.
I'm not responsible for any damage or corrupted data that may occur using this.

