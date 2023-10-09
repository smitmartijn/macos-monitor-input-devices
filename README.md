# Monitoring input devices on macOS

Martijn Smit <https://x.com/smitmartijn>

Monitors the attached HID devices on macOS and prints information about them when they are connected and disconnected.

## Usage
```
make
./macos-monitor-input-devices
```

## Example output

```
New mouse device 0x7fc691b04ca0
 - Product: DELL Laser Mouse (4D51)
 - Vendor:  (0461)
 - Serial:
 - Transport type: USB
 - Usage Key: 2 - Usage Page Key: 1
New keyboard device 0x7fc691b05a60
 - Product: Razer BlackWidow Elite (0228)
 - Vendor: Razer (1532)
 - Serial:
 - Transport type: USB
 - Usage Key: 6 - Usage Page Key: 1
New keyboard device 0x7fc691b04b40
 - Product: Matias RGB Backlit Wired Keyboard (024F)
 - Vendor: Matias Keyboard (05AC)
 - Serial:
 - Transport type: USB
 - Usage Key: 6 - Usage Page Key: 1


Removed device: 0x7fc691b04ca0
 - Product: DELL Laser Mouse (4D51)
 - Vendor:  (0461)
 - Serial:
 - Transport type: USB
 - Usage Key: 2 - Usage Page Key: 1
New mouse device 0x7fc691f04370
 - Product: DELL Laser Mouse (4D51)
 - Vendor:  (0461)
 - Serial:
 - Transport type: USB
 - Usage Key: 2 - Usage Page Key: 1
```

2023-10-09: Initial release
