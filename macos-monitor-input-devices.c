/**
 *
 *	Monitoring input devices on macOS
 *	Martijn Smit <https://x.com/smitmartijn>
 *
 *  Monitors the attached HID devices on macOS and prints information about them when they are connected and disconnected.
 *
 *	Usage: make && ./macos-monitor-input-devices
 *
 * 2023-10-09: Initial release
 */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
  CFTypeRef ref;
  int32_t value;

  ref = IOHIDDeviceGetProperty(device, key);
  if (ref)
  {
    if (CFGetTypeID(ref) == CFNumberGetTypeID())
    {
      CFNumberGetValue((CFNumberRef)ref, kCFNumberSInt32Type, &value);
      return value;
    }
  }
  return 0;
}

static int get_string_property(IOHIDDeviceRef device, CFStringRef prop, wchar_t *buf, size_t len)
{
  CFStringRef str;

  if (!len)
    return 0;

  str = (CFStringRef)IOHIDDeviceGetProperty(device, prop);

  buf[0] = 0;

  if (str)
  {
    CFIndex str_len = CFStringGetLength(str);
    CFRange range;
    CFIndex used_buf_len;
    CFIndex chars_copied;

    len--;

    range.location = 0;
    range.length = ((size_t)str_len > len) ? len : (size_t)str_len;
    chars_copied = CFStringGetBytes(str,
                                    range,
                                    kCFStringEncodingUTF32LE,
                                    (char)'?',
                                    FALSE,
                                    (UInt8 *)buf,
                                    len * sizeof(wchar_t),
                                    &used_buf_len);

    if (chars_copied <= 0)
      buf[0] = 0;
    else
      buf[chars_copied] = 0;

    return 0;
  }
  else
    return -1;
}

static unsigned short get_vendor_id(IOHIDDeviceRef device)
{
  return get_int_property(device, CFSTR(kIOHIDVendorIDKey));
}

static unsigned short get_product_id(IOHIDDeviceRef device)
{
  return get_int_property(device, CFSTR(kIOHIDProductIDKey));
}

static unsigned short get_usage(IOHIDDeviceRef device)
{
  return get_int_property(device, CFSTR(kIOHIDPrimaryUsageKey));
}

static unsigned short get_usage_page(IOHIDDeviceRef device)
{
  return get_int_property(device, CFSTR(kIOHIDPrimaryUsagePageKey));
}

static int get_serial_number(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
  return get_string_property(device, CFSTR(kIOHIDSerialNumberKey), buf, len);
}

static int get_manufacturer_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
  return get_string_property(device, CFSTR(kIOHIDManufacturerKey), buf, len);
}

static int get_product_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
  return get_string_property(device, CFSTR(kIOHIDProductKey), buf, len);
}

static int get_transport_type(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
  return get_string_property(device, CFSTR(kIOHIDTransportKey), buf, len);
}

static void print_device_info(IOHIDDeviceRef deviceRef)
{
  wchar_t vendorName[256], productName[256], serialNumber[256], transportType[256];

  get_manufacturer_string(deviceRef, vendorName, sizeof(vendorName));
  get_product_string(deviceRef, productName, sizeof(productName));
  get_serial_number(deviceRef, serialNumber, sizeof(serialNumber));
  get_transport_type(deviceRef, transportType, sizeof(transportType));

  printf(" - Product: %ls (%04hX)\n", productName, get_product_id(deviceRef));
  printf(" - Vendor: %ls (%04hX)\n", vendorName, get_vendor_id(deviceRef));
  printf(" - Serial: %ls\n", serialNumber);
  printf(" - Transport type: %ls\n", transportType);
  printf(" - Usage Key: %d - Usage Page Key: %d\n", get_usage(deviceRef), get_usage_page(deviceRef));
}

static void HIDDeviceRemoved(__unused void *context, __unused IOReturn result, __unused void *sender, IOHIDDeviceRef deviceRef)
{
  printf("Removed device: %p\n", (void *)deviceRef);

  print_device_info(deviceRef);

  IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeNone);
}

static void HIDDeviceConnected(__unused void *context, __unused IOReturn result, __unused void *sender, IOHIDDeviceRef deviceRef)
{
  CFNumberRef usagePageValue = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDPrimaryUsagePageKey));
  CFNumberRef usageValue = (CFNumberRef)IOHIDDeviceGetProperty(deviceRef, CFSTR(kIOHIDPrimaryUsageKey));

  if (!usagePageValue || !usageValue)
  {
    IOHIDDeviceClose(deviceRef, kIOHIDOptionsTypeNone);
    return;
  }

  int32_t usagePage;
  int32_t usage;

  CFNumberGetValue(usagePageValue, kCFNumberSInt32Type, &usagePage);
  CFNumberGetValue(usageValue, kCFNumberSInt32Type, &usage);

  if (usagePage != kHIDPage_GenericDesktop)
    return;

  if (usage != kHIDUsage_GD_Keyboard && usage != kHIDUsage_GD_Keypad && usage != kHIDUsage_GD_Mouse && usage != kHIDUsage_GD_Pointer)
    return;

  if (IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
  {
    printf("Couldn't open device\n");
    return;
  }

  if (usage == kHIDUsage_GD_Keyboard || usage == kHIDUsage_GD_Keypad)
  {
    printf("New keyboard device %p\n", (void *)deviceRef);
  }

  if (usage == kHIDUsage_GD_Mouse || usage == kHIDUsage_GD_Pointer)
  {
    printf("New mouse device %p\n", (void *)deviceRef);
  }

  print_device_info(deviceRef);

  IOHIDDeviceScheduleWithRunLoop(deviceRef, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
}

int main()
{
  // Find all HID Devices
  IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  IOHIDManagerSetDeviceMatching(hidManager, NULL);

  IOHIDManagerRegisterDeviceMatchingCallback(hidManager, &HIDDeviceConnected, NULL);
  IOHIDManagerRegisterDeviceRemovalCallback(hidManager, &HIDDeviceRemoved, NULL);

  IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
  IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

  // Run the run loop
  CFRunLoopRun();

  return 0;
}