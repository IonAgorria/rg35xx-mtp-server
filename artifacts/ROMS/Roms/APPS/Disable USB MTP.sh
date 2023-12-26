#!/bin/sh

if [[ -n "$OLD_SCRIPT_PATH" ]]; then
  #This is second run after copying ourselves
  rm -f "$OLD_SCRIPT_PATH"
  sync
  mount -o remount,ro /mnt/SDCARD
  mount -o remount,ro /mnt/mmc
  reboot -f
  exit 0
fi

export OLD_SCRIPT_PATH="$0"
APPS_DIR=$(dirname "$OLD_SCRIPT_PATH")

mount -o remount,rw /misc
if [ -f /misc/enableMTP ]; then
	rm /misc/enableMTP
	NEW_SCRIPT_PATH="$APPS_DIR/Enable USB MTP.sh"
else
	touch /misc/enableMTP
	NEW_SCRIPT_PATH="$APPS_DIR/Disable USB MTP.sh"
fi
cp "$OLD_SCRIPT_PATH" "$NEW_SCRIPT_PATH"
sync
mount -o remount,ro /misc
exec "$NEW_SCRIPT_PATH"
exit 0

