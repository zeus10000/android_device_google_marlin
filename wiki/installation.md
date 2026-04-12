# Install LineageOS 23.2 on Google Pixel XL (marlin)

## Prerequisites
- Google Pixel XL (marlin) with unlocked bootloader
- USB cable
- Latest platform-tools (adb, fastboot)
- Working firmware from factory image 8.1.0 (OPM1.171019.021)

## Firmware requirement
Device must have firmware from Google factory image **8.1.0 (OPM1.171019.021)** or later.
Vendor image is included in the LineageOS build.

## Installation steps

### 1. Boot into fastboot mode
Power off the device, then hold **Power + Volume Down** until the fastboot screen appears.

### 2. Flash LineageOS Recovery
```
fastboot flash boot lineage-23.2-marlin-recovery.img
```

### 3. Reboot into recovery
Use volume keys to select "Recovery mode" and press Power.

### 4. Factory reset (first install only)
In recovery, select **Factory Reset** → **Format data/factory reset**.

### 5. Sideload LineageOS
Select **Apply update** → **Apply from ADB**, then:
```
adb sideload lineage-23.2-*-marlin-signed.zip
```

### 6. (Optional) Install Google Apps
Reboot to recovery after LineageOS install, then sideload GApps:
```
adb sideload MindTheGapps-*.zip
```

### 7. Reboot
Select **Reboot system now**.

## Notes
- LineageOS Recovery is the recommended recovery solution
- A/B slot device: LineageOS manages slot switching automatically
- SELinux is in Enforcing mode
