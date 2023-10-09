
all: macos-monitor-input-devices

LIBS=-framework IOKit -framework CoreFoundation

macos-monitor-input-devices: macos-monitor-input-devices.c
	$(CC) $(CFLAGS) macos-monitor-input-devices.c -o macos-monitor-input-devices $(LIBS)

clean:
	rm macos-monitor-input-devices

