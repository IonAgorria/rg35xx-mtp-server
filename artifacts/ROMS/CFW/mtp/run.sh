#!/bin/sh

if [ ! -f "/misc/enableMTP" ] || [ ! -e "/dev/mtp_usb" ]; then 
  exit 0
fi

export MTP_MANUFACTURER="Anbernic"
export MTP_DEVICE="RG35XX"

#Add or modify entries as desired
export MTP_ENTRY_LEN=2
export MTP_ENTRY_1_PATH="/mnt/mmc"
export MTP_ENTRY_1_NAME="TF1 MMC"
export MTP_ENTRY_1_REMOVABLE="0"
export MTP_ENTRY_2_PATH="/mnt/SDCARD"
export MTP_ENTRY_2_NAME="TF2 SDCARD"
export MTP_ENTRY_2_REMOVABLE="1"

while true; do
  ./mtp-server > /dev/null
  sleep 1
done
