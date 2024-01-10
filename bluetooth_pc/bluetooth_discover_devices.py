
import bluetooth
from address_devices import *
import sys



def discover_all_devices():
    # iscover_devices (duration=8, flush_cache=True, lookup_names=False,lookup_class=False, device_id=-1):
    nearby_devices = bluetooth.discover_devices(duration=8, lookup_names=True, lookup_class=True, device_id=-1)
    return nearby_devices

def find_device_by_address(target_address):
    nearby_devices = bluetooth.discover_devices(duration=8, lookup_names=True)

    for addr, name in nearby_devices:
        if addr == target_address:
            return addr, name

    return None


def main():
    if sys.argv[1] == "--cam":
        target_address = address_devices.CAM_ESP32_BT_ADDRESS
    elif  sys.argv[1] == "--nfc":
        target_address = address_devices.NFC_ESP32_BT_ADDRESS 
    else: 
        print("ERRORE. Parametri validi: --cam, --nfc")
        exit(2)

    result = find_device_by_address(target_address)

    if result:
        addr, name = result
        print(f"Device found - Address: {addr}, Name: {name}")
        exit(0)
    else:
        print("Device not found.")
        exit(1)
if __name__ == "__main__":
    main()
