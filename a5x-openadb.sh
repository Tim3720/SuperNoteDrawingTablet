#!/bin/bash

echo "rebooting to recovery"
adb reboot recovery

ANSWER=0
while [ "$ANSWER" != "1" ]; do
    sleep 2
    ANSWER=$(adb devices | grep rockchipplatform -c)
done

echo "device online, patching"

adb shell busybox mount -o rw,seclabel,relatime,data=ordered,inode_readahead_blks=8 /dev/block/by-name/system /system
adb shell sed -i "s/ro.debuggable=0/ro.debuggable=1/" /system/etc/prop.default

echo "rebooting to system"
adb reboot
