#!/system/bin/sh

if [ ! -f /misc/enableMTP ] && [ ! -f /misc/enableADB ]; then
  exit 0;
fi

usb_gandroid0="/sys/class/android_usb/android0"

echo 0 > /sys/monitor/usb_port/config/run
echo 1 > /sys/monitor/usb_port/config/idpin_debug
echo "USB_A_OUT" > /sys/monitor/usb_port/config/usb_con_msg
echo "USB_B_IN" > /sys/monitor/usb_port/config/usb_con_msg

echo '0' > "$usb_gandroid0/enable"
echo "Anbernic" > "$usb_gandroid0/iManufacturer"
echo "RG35XX" > "$usb_gandroid0/iProduct"
echo "10d6" > "$usb_gandroid0/idVendor"

if [ -f /misc/enableADB ]; then
    echo "4e42" > "$usb_gandroid0/idProduct"
    echo "mtp,adb" > "$usb_gandroid0/functions"
else
    echo "4e41" > "$usb_gandroid0/idProduct"
    echo "mtp" > "$usb_gandroid0/functions"
fi

echo '1' > "$usb_gandroid0/enable"

# Enable ADB for debugging
if [ -f /misc/enableADB ]; then
    /system/bin/start adbd
fi
